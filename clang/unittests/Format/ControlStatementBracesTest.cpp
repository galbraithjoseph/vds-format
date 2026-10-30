//===- ControlStatementBracesTest.cpp -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FormatTestBase.h"

namespace clang {
namespace format {
namespace test {
namespace {

using Policy = FormatStyle::ControlStatementBracesStyle;

class ControlStatementBracesTest : public FormatTestBase {
protected:
  FormatStyle getDefaultStyle() const override {
    auto Style = getLLVMStyle();
    Style.BreakBeforeBraces = FormatStyle::BS_Allman;
    Style.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
    Style.AllowShortLoopsOnASingleLine = false;
    Style.AllowShortBlocksOnASingleLine = FormatStyle::SBS_Never;
    Style.ControlStatementBraces = {Policy::SingleStatement, Policy::Consistent,
                                    Policy::Require};
    return Style;
  }

  // Every case checks the actual transformation and second-pass stability.
  void check(StringRef Expected, StringRef Input,
             std::optional<FormatStyle> Style = {}) {
    EXPECT_EQ(Expected, format(Input, Style));
    EXPECT_EQ(Expected, format(Expected, Style));
    EXPECT_EQ(0u, ReplacementCount) << "Already formatted code needs no edits";
  }
};

TEST_F(ControlStatementBracesTest, SimpleAndMixedChains) {
  check("if (ready)\n  run();\nelse\n  wait();",
        "if(ready){run();}else{wait();}");
  check("if (a)\n  f();\nelse if (b)\n  g();\nelse\n  h();",
        "if(a){f();}else if(b)g();else{h();}");
  check("if (a)\n{\n  f();\n}\nelse if (b)\n{\n  g();\n  h();\n}\n"
        "else\n{\n  i();\n}",
        "if(a)f();else if(b){g();h();}else i();");
  check("if (a)\n{\n  f();\n  g();\n}\nelse if (b)\n{\n  h();\n}\n"
        "else if (c)\n{\n  i();\n}\nelse\n{\n  j();\n}",
        "if(a){f();g();}else if(b)h();else if(c){i();}else j();");
}

TEST_F(ControlStatementBracesTest, PreprocessorBranchesDoNotRepeatInsertion) {
  const std::string Conditional =
      "#if X\nint x;\n#elif Y\nint y;\n#else\nint z;\n#endif\n";
  const std::string Input = "if(a)f();while(b)g();";
  const std::string Expected = "if (a)\n  f();\nwhile (b)\n  g();\n";
  for (bool Before : {false, true}) {
    const std::string Source = Before ? Input + "\n" + Conditional
                                      : Conditional + Input + "\n";
    const std::string Result = Before ? Expected + Conditional
                                      : Conditional + Expected;
    check(Result, Source);
    auto Style = getDefaultStyle();
    Style.FunctionDeclarationSeparatorColumn = 80;
    check(Result, Source, Style);
    Style.ControlStatementBraces.Mode = Policy::Always;
    const std::string Braced =
        "if (a)\n{\n  f();\n}\nwhile (b)\n{\n  g();\n}\n";
    check(Before ? Braced + Conditional : Conditional + Braced, Source, Style);
  }
}

TEST_F(ControlStatementBracesTest, IndependentBranches) {
  auto Style = getDefaultStyle();
  Style.ControlStatementBraces.IfChain = Policy::Independent;
  check("if (a)\n{\n  f();\n  g();\n}\nelse\n  h();",
        "if(a){f();g();}else{h();}", Style);
}

TEST_F(ControlStatementBracesTest, FinalConditionWrapping) {
  auto Style = getDefaultStyle();
  Style.ColumnLimit = 30;
  check("if (firstCondition &&\n    secondCondition)\n{\n  f();\n}\n"
        "else\n{\n  g();\n}",
        "if(firstCondition && secondCondition)f();else g();", Style);
  check("if (a)\n{\n  f();\n}\nelse if (firstCondition &&\n"
        "         secondCondition)\n{\n  g();\n}\nelse\n{\n  h();\n}",
        "if(a)f();else if(firstCondition && secondCondition)g();else h();",
        Style);
  // A line break in the input alone is not a wrapped-condition trigger.
  check("if (a && b)\n  f();\nelse\n  g();", "if(a &&\nb){f();}else{g();}",
        Style);
  Style.ControlStatementBraces.WrappedCondition = Policy::Ignore;
  check("if (firstCondition &&\n    secondCondition)\n  f();\nelse\n  g();",
        "if(firstCondition && secondCondition){f();}else{g();}", Style);
}

TEST_F(ControlStatementBracesTest, InitializerWrapping) {
  auto Style = getDefaultStyle();
  Style.ColumnLimit = 30;
  const auto Result = format(
      "if(auto value = getValue(); firstCondition && secondCondition)f();"
      "else g();",
      Style);
  EXPECT_NE(std::string::npos, Result.find("\n{\n  f();"));
  EXPECT_NE(std::string::npos, Result.find("else\n{\n  g();"));
  EXPECT_EQ(Result, format(Result, Style));
  const auto Chain =
      format("if(a)f();else if(auto value = getValue(); firstCondition && "
             "secondCondition)g();else h();",
             Style);
  EXPECT_NE(std::string::npos, Chain.find("if (a)\n{\n  f();"));
  EXPECT_NE(std::string::npos, Chain.find("else\n{\n  h();"));
  EXPECT_EQ(Chain, format(Chain, Style));
}

TEST_F(ControlStatementBracesTest, MultilineStatementIsStillOneStatement) {
  auto Style = getDefaultStyle();
  Style.ColumnLimit = 30;
  check("if (a)\n  veryLongFunctionName(\n      firstArgument,\n      "
        "secondArgument);\nelse\n  g();",
        "if(a){veryLongFunctionName(firstArgument,secondArgument);}else{g();}",
        Style);
}

TEST_F(ControlStatementBracesTest, Loops) {
  check("while (a)\n  f();", "while(a){f();}");
  check("for (int i = 0; i < n; ++i)\n  f();", "for(int i=0;i<n;++i){f();}");
  check("for (auto x : xs)\n  f(x);", "for(auto x:xs){f(x);}");
  check("do\n  f();\nwhile (a);", "do{f();}while(a);");
  check("while (a)\n{\n  f();\n  g();\n}", "while(a){f();g();}");
  auto Style = getDefaultStyle();
  Style.ColumnLimit = 30;
  check("while (firstCondition &&\n       secondCondition)\n{\n  f();\n}",
        "while(firstCondition && secondCondition)f();", Style);
  check("do\n{\n  f();\n} while (firstCondition &&\n         secondCondition);",
        "do f();while(firstCondition && secondCondition);", Style);
}

TEST_F(ControlStatementBracesTest, EmptyBodies) {
  check("if (a)\n{\n}\nelse\n{\n  f();\n}", "if(a){}else f();");
  check("while (a)\n  ;", "while(a);");
  check("do\n{\n} while (a);", "do{}while(a);");
}

TEST_F(ControlStatementBracesTest, DanglingElseAndNestedControlFlow) {
  check("if (a)\n{\n  if (b)\n    f();\n}\nelse\n{\n  g();\n}",
        "if(a){if(b)f();}else g();");
  check("if (a)\n  while (b)\n    f();", "if(a){while(b){f();}}");
  check("if (a)\n  if (b)\n    f();\n  else\n    g();",
        "if(a){if(b){f();}else{g();}}");
  check("if (a)\n{\n  while (b)\n  {\n    if (c)\n      f();\n  }\n}\n"
        "else\n{\n  g();\n}",
        "if(a){while(b){if(c)f();}}else g();");
}

TEST_F(ControlStatementBracesTest, CommentsAttributesAndDirectives) {
  check("if (a)\n{\n  // Keep this body.\n  f();\n}\nelse\n{\n  g();\n}",
        "if(a){\n// Keep this body.\nf();\n}else g();");
  check("if (a) [[likely]]\n{\n  f();\n}\nelse\n{\n  g();\n}",
        "if(a) [[likely]] {f();}else g();");
  check("if (a)\n{\n#if ENABLED\n  f();\n#endif\n}\nelse\n{\n  g();\n}",
        "if(a){\n#if ENABLED\nf();\n#endif\n}else g();");
  check("if consteval\n{\n  f();\n}\nelse\n{\n  g();\n}",
        "if consteval {f();}else{g();}");
  check("// clang-format off\nif(a){f();}else{g();}\n// clang-format on",
        "// clang-format off\nif(a){f();}else{g();}\n// clang-format on");
}

TEST_F(ControlStatementBracesTest, LeaveAndAlways) {
  auto Style = getDefaultStyle();
  Style.ControlStatementBraces.Mode = Policy::Leave;
  check("if (a)\n{\n  f();\n}\nelse\n  g();", "if(a){f();}else g();", Style);
  Style.ControlStatementBraces.Mode = Policy::Always;
  check("if (a)\n{\n  f();\n}\nelse if (b)\n{\n  g();\n}\nelse\n{\n  h();\n}",
        "if(a)f();else if(b)g();else h();", Style);
  check("do\n{\n  f();\n} while (a);", "do f();while(a);", Style);
}

TEST_F(ControlStatementBracesTest, AttachedBraceBoundaryIsStable) {
  auto Style = getDefaultStyle();
  Style.BreakBeforeBraces = FormatStyle::BS_Attach;
  Style.ColumnLimit = 25;
  const auto Result = format("if(firstCondition && b)f();else g();", Style);
  EXPECT_EQ(Result, format(Result, Style));
}

TEST_F(ControlStatementBracesTest, NestedElseChainsRemainStable) {
  check(
      "if (a)\n{\n  f();\n}\nelse\n{\n  if (b)\n  {\n    g();\n    h();\n  }\n"
      "  else\n  {\n    i();\n  }\n}",
      "if(a){f();}else{if(b){g();h();}else i();}");
}

TEST_F(ControlStatementBracesTest, LanguagesAndDefaultBehavior) {
  auto Style = getDefaultStyle();
  for (auto Language : {FormatStyle::LK_C, FormatStyle::LK_ObjC}) {
    Style.Language = Language;
    check("if (a)\n  f();\nelse\n  g();", "if(a){f();}else{g();}", Style);
  }
  Style.Language = FormatStyle::LK_JavaScript;
  check("if (a)\n{\n  f();\n}\nelse\n{\n  g();\n}", "if(a){f();}else{g();}",
        Style);
  Style = getLLVMStyle();
  check("if (a) {\n  f();\n} else\n  g();", "if(a){f();}else g();", Style);
}

TEST_F(ControlStatementBracesTest, RangeDoesNotRemoveOnlyOneBrace) {
  auto Style = getDefaultStyle();
  StringRef Code = "if (a)\n{\n  f();\n}\n";
  EXPECT_EQ(Code, format(Code, Style, SC_ExpectComplete,
                         {tooling::Range(Code.find('}'), 1)}));
}

TEST_F(ControlStatementBracesTest, OtherTransformationsPrecedeWrapDecision) {
  auto Style = getDefaultStyle();
  Style.ColumnLimit = 30;
  Style.RemoveParentheses = FormatStyle::RPS_MultipleParentheses;
  const auto Result =
      format("if (((firstCondition && b))) { f(); } else { g(); }", Style);
  EXPECT_EQ(Result, format(Result, Style));
  EXPECT_EQ(0u, ReplacementCount);
}

TEST_F(ControlStatementBracesTest, PasteRangeMustContainBothBraceEdits) {
  const auto Style = getDefaultStyle();
  for (StringRef Newline : {"\n", "\r\n"}) {
    std::string Code;
    for (StringRef Line :
         {"void f()", "{", "  if (a)", "  {", "    return;", "  }", "}"})
      Code += (Line + Newline).str();
    const auto Begin = Code.find("if");
    const auto Close = Code.find('}', Begin);
    // Paste/on-type requests may end before the closing brace. Affected-line
    // expansion must not turn this into two brace edits straddling the range.
    for (auto End : {Code.find("return") + 7, Close})
      EXPECT_EQ(Code, format(Code, Style, SC_ExpectComplete,
                             {tooling::Range(Begin, End - Begin)}));
    std::string Expected;
    for (StringRef Line : {"void f()", "{", "  if (a)", "    return;", "}"})
      Expected += (Line + Newline).str();
    EXPECT_EQ(Expected, format(Code, Style, SC_ExpectComplete,
                               {tooling::Range(Begin, Close + 1 - Begin)}));
  }
}

TEST_F(ControlStatementBracesTest, PasteRangeMustContainBothInsertedBraces) {
  auto Style = getDefaultStyle();
  Style.ControlStatementBraces.Mode = Policy::Always;
  StringRef Code = "void f()\n{\n  if (a)\n    return;\n}\n";
  const auto Begin = Code.find("if");
  const auto Body = Code.find("return");
  EXPECT_EQ(Code, format(Code, Style, SC_ExpectComplete,
                         {tooling::Range(Begin, Body - Begin)}));
  EXPECT_EQ("void f()\n{\n  if (a)\n  {\n    return;\n  }\n}\n",
            format(Code, Style, SC_ExpectComplete,
                   {tooling::Range(Begin, Body + 7 - Begin)}));
}

TEST_F(ControlStatementBracesTest, PasteAllowsWhitespaceOutsideBraceRange) {
  auto Style = getDefaultStyle();
  Style.UseTab = FormatStyle::UT_Always;
  Style.IndentWidth = Style.TabWidth = 4;
  Style.LineEnding = FormatStyle::LE_LF;
  StringRef Code = "void f()\r\n{\r\n\tif (!connection)\r\n\t{\r\n"
                   "\t\treturn;\r\n\t}\r\n\r\n\tnext();\r\n}\r\n";
  const auto Begin = Code.find("if");
  const auto End = Code.find('}', Begin) + 1;
  const auto Result = format(Code, Style, SC_ExpectComplete,
                             {tooling::Range(Begin, End - Begin)});
  EXPECT_EQ(1, llvm::count(Result, '{'));
  EXPECT_EQ(1, llvm::count(Result, '}'));
  EXPECT_NE(std::string::npos, Result.find("if (!connection)\n\t\treturn;"));
}

} // namespace
} // namespace test
} // namespace format
} // namespace clang
