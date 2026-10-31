"""Brace completion keeps the cursor after the opening brace in both output modes."""
import json
import subprocess
import sys
import xml.etree.ElementTree as ET

formatter, filename = sys.argv[1:]
cases = [
    "void f() {}",
    "void f() { if (ready) {} }",
    "void f() { while (ready) {} }",
    "void f() { for (;;) {} }",
    "auto fn = []() {};",
    "class C {};",
    "namespace N {}",
]
for newline in ["\n", "\r\n"]:
    style = json.dumps({"BasedOnStyle": "LLVM", "BreakBeforeBraces": "Allman",
                        "AllowShortFunctionsOnASingleLine": "None",
                        "AllowShortBlocksOnASingleLine": "Never",
                        "AllowShortLambdasOnASingleLine": "None",
                        "LineEnding": "CRLF" if newline == "\r\n" else "LF"})
    for case in cases:
        for includes in ["", "#include <z>" + newline + "#include <a>" + newline]:
            source = (includes + case + newline).encode()
            cursor = source.index(b"{}") + 1
            for partial in [False, True]:
                args = [formatter, "--style=" + style, "--assume-filename=" + filename,
                        "--cursor=" + str(cursor)]
                if partial:
                    args += ["--offset=" + str(cursor - 1), "--length=2"]
                xml = subprocess.check_output(args + ["--output-replacements-xml"], input=source)
                root = ET.fromstring(xml)
                formatted = source
                for edit in reversed(root.findall("replacement")):
                    offset, length = int(edit.attrib["offset"]), int(edit.attrib["length"])
                    formatted = formatted[:offset] + (edit.text or "").encode() + formatted[offset + length:]
                mapped = int(root.findtext("cursor"))
                assert formatted[mapped - 1:mapped] == b"{", (case, formatted, mapped)
                plain = subprocess.check_output(args, input=source)
                header, text = plain.split(b"\n", 1)
                assert text == formatted
                assert json.loads(header)["Cursor"] == mapped
print("PASS: empty brace cursor stays after { across constructs, ranges, line endings, and include sorting.")
