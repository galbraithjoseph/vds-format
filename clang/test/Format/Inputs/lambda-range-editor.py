"""Partial lambda formatting must remain valid when an editor filters edits."""
import json
import subprocess
import sys
import xml.etree.ElementTree as ET

formatter, filename = sys.argv[1:]
for newline in [b"\n", b"\r\n"]:
    style = json.dumps({"BasedOnStyle": "LLVM", "BreakBeforeBraces": "Allman",
                        "BraceWrapping": {"BeforeLambdaBody": True},
                        "AllowShortLambdasOnASingleLine": "All",
                        "LambdaBodyIndentation": "OuterScope",
                        "UseTab": "Always", "TabWidth": 4, "IndentWidth": 4,
                        "LineEnding": "CRLF" if newline == b"\r\n" else "LF"})
    source = newline.join([b"void f()", b"{",
        b"\tfoo([thisRef = BlahBlahBlah(this, false), this]", b"\t{",
        b"\t\tRunTimers();", b"\t});", b"}"])
    semi = source.index(b";")
    brace = source.index(b"{", source.index(b"foo"))
    for start in [semi, brace]:
        end = semi + 1
        result = subprocess.check_output([formatter, "--style=" + style,
            "--assume-filename=" + filename, "--output-replacements-xml",
            "--offset=" + str(start), "--length=" + str(end - start)], input=source)
        edited = source
        for edit in reversed(ET.fromstring(result).findall("replacement")):
            offset, length = int(edit.attrib["offset"]), int(edit.attrib["length"])
            if offset >= start and offset + length <= end:
                edited = edited[:offset] + (edit.text or "").encode() + edited[offset + length:]
        assert edited == source, edited
print("PASS: on-type range-filtered lambda edits preserve brace and body lines for LF and CRLF.")
