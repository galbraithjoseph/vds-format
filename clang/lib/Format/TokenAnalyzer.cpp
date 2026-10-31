//===--- TokenAnalyzer.cpp - Analyze Token Streams --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements an abstract TokenAnalyzer and associated helper
/// classes. TokenAnalyzer can be extended to generate replacements based on
/// an annotated and pre-processed token stream.
///
//===----------------------------------------------------------------------===//

#include "TokenAnalyzer.h"
#include "AffectedRangeManager.h"
#include "Encoding.h"
#include "FormatToken.h"
#include "FormatTokenLexer.h"
#include "TokenAnnotator.h"
#include "UnwrappedLineParser.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Format/Format.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "format-formatter"

namespace clang {
namespace format {

// FIXME: Instead of printing the diagnostic we should store it and have a
// better way to return errors through the format APIs.
class FatalDiagnosticConsumer : public DiagnosticConsumer {
public:
  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    if (DiagLevel == DiagnosticsEngine::Fatal) {
      Fatal = true;
      llvm::SmallVector<char, 128> Message;
      Info.FormatDiagnostic(Message);
      llvm::errs() << Message << "\n";
    }
  }

  bool fatalError() const { return Fatal; }

private:
  bool Fatal = false;
};

std::unique_ptr<Environment>
Environment::make(StringRef Code, StringRef FileName,
                  ArrayRef<tooling::Range> Ranges, unsigned FirstStartColumn,
                  unsigned NextStartColumn, unsigned LastStartColumn) {
  auto Env = std::make_unique<Environment>(Code, FileName, FirstStartColumn,
                                           NextStartColumn, LastStartColumn);
  FatalDiagnosticConsumer Diags;
  Env->SM.getDiagnostics().setClient(&Diags, /*ShouldOwnClient=*/false);
  SourceLocation StartOfFile = Env->SM.getLocForStartOfFile(Env->ID);
  for (const tooling::Range &Range : Ranges) {
    SourceLocation Start = StartOfFile.getLocWithOffset(Range.getOffset());
    SourceLocation End = Start.getLocWithOffset(Range.getLength());
    Env->CharRanges.push_back(CharSourceRange::getCharRange(Start, End));
  }
  // Validate that we can get the buffer data without a fatal error.
  Env->SM.getBufferData(Env->ID);
  if (Diags.fatalError())
    return nullptr;
  return Env;
}

Environment::Environment(StringRef Code, StringRef FileName,
                         unsigned FirstStartColumn, unsigned NextStartColumn,
                         unsigned LastStartColumn)
    : VirtualSM(new SourceManagerForFile(FileName, Code)), SM(VirtualSM->get()),
      ID(VirtualSM->get().getMainFileID()), FirstStartColumn(FirstStartColumn),
      NextStartColumn(NextStartColumn), LastStartColumn(LastStartColumn) {}

TokenAnalyzer::TokenAnalyzer(const Environment &Env, const FormatStyle &Style)
    : Style(Style), LangOpts(getFormattingLangOpts(Style)), Env(Env),
      AffectedRangeMgr(Env.getSourceManager(), Env.getCharRanges()),
      Encoding(encoding::detectEncoding(
          Env.getSourceManager().getBufferData(Env.getFileID()))) {
  LLVM_DEBUG(
      llvm::dbgs() << "File encoding: "
                   << (Encoding == encoding::Encoding_UTF8 ? "UTF8" : "unknown")
                   << "\n");
  LLVM_DEBUG(llvm::dbgs() << "Language: " << getLanguageName(Style.Language)
                          << "\n");
}

std::pair<tooling::Replacements, unsigned>
TokenAnalyzer::process(bool SkipAnnotation) {
  tooling::Replacements Result;
  llvm::SpecificBumpPtrAllocator<FormatToken> Allocator;
  IdentifierTable IdentTable(LangOpts);
  FormatTokenLexer Lex(Env.getSourceManager(), Env.getFileID(),
                       Env.getFirstStartColumn(), Style, Encoding, Allocator,
                       IdentTable);
  ArrayRef<FormatToken *> Toks(Lex.lex());
  SmallVector<FormatToken *, 10> Tokens(Toks);

  unsigned Penalty = 0;
  unsigned Run = 0;
  bool HasError = false;
  OnFinishRun = [&]() {
    if (HasError)
      return;
    LLVM_DEBUG(llvm::dbgs() << "Run " << Run << "...\n");
    SmallVector<AnnotatedLine *, 16> AnnotatedLines;
    AnnotatedLines.reserve(UnwrappedLines.size());

    TokenAnnotator Annotator(Style, Lex.getKeywords());
    for (const UnwrappedLine &Line : UnwrappedLines) {
      AnnotatedLines.push_back(new AnnotatedLine(Line));
      if (!SkipAnnotation)
        Annotator.annotate(*AnnotatedLines.back());
    }

    auto MarkFunctionBodies =
        [&](auto &&Self, SmallVectorImpl<AnnotatedLine *> &Lines) -> void {
      for (unsigned I = 0; I < Lines.size(); ++I) {
        auto *Line = Lines[I];
        for (auto *Tok = Line->First; Tok; Tok = Tok->Next) {
          Line->HasFunctionBody |= Tok->is(TT_FunctionLBrace);
          Self(Self, Tok->Children);
        }
        unsigned Next = I + 1;
        while (Next < Lines.size() && Lines[Next]->First->is(tok::comment))
          ++Next;
        // K&R parameter declarations can occupy separate unwrapped lines
        // between the signature and its body.
        if (!Line->endsWith(tok::semi)) {
          while (Next < Lines.size() && Lines[Next]->Level == Line->Level &&
                 !Lines[Next]->InPPDirective &&
                 !Lines[Next]->MightBeFunctionDecl &&
                 Lines[Next]->endsWith(tok::semi))
            ++Next;
        }
        if (Next < Lines.size())
          Line->HasFunctionBody |= Lines[Next]->First->is(tok::l_brace);
      }
    };
    MarkFunctionBodies(MarkFunctionBodies, AnnotatedLines);

    std::pair<tooling::Replacements, unsigned> RunResult =
        analyze(Annotator, AnnotatedLines, Lex);

    LLVM_DEBUG({
      llvm::dbgs() << "Replacements for run " << Run << ":\n";
      for (const tooling::Replacement &Fix : RunResult.first)
        llvm::dbgs() << Fix.toString() << "\n";
    });
    for (AnnotatedLine *Line : AnnotatedLines)
      delete Line;
    ++Run;

    Penalty += RunResult.second;
    for (const auto &R : RunResult.first) {
      auto Err = Result.add(R);
      // FIXME: better error handling here. For now, simply return an empty
      // Replacements to indicate failure.
      if (Err) {
        llvm::errs() << llvm::toString(std::move(Err)) << "\n";
        HasError = true;
        return;
      }
    }
  };
  UnwrappedLineParser Parser(Env.getSourceManager(), Style, Lex.getKeywords(),
                             Env.getFirstStartColumn(), Tokens, *this,
                             Allocator, IdentTable);
  Parser.parse();
  if (HasError)
    return {tooling::Replacements(), 0};
  return {Result, Penalty};
}

void TokenAnalyzer::consumeUnwrappedLine(const UnwrappedLine &TheLine) {
  UnwrappedLines.push_back(TheLine);
}

void TokenAnalyzer::finishRun() {
  OnFinishRun();
  UnwrappedLines.clear();
}

} // end namespace format
} // end namespace clang
