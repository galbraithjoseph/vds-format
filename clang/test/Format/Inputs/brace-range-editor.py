"""Brace removals must survive editors that apply only in-range XML edits."""

import json
import subprocess
import sys
import xml.etree.ElementTree as ET


formatter, filename = sys.argv[1:]
style = json.dumps({
    "BasedOnStyle": "LLVM",
    "UseTab": "Always",
    "TabWidth": 4,
    "IndentWidth": 4,
    "LineEnding": "LF",
    "BreakBeforeBraces": "Allman",
    "ControlStatementBraces": {
        "Mode": "SingleStatement",
        "IfChain": "Consistent",
        "WrappedCondition": "Require",
    },
})

for newline in (b"\n", b"\r\n"):
    source = newline.join([
        b"void f()", b"{", b"\tif (!ready)", b"\t{",
        b"\t\tperformOperation(firstArgument, secondArgument);",
        b"\t}", b"        next();", b"    }", b"",
    ])
    start = source.index(b"if")
    close = source.index(b"}") + 1
    for end in sorted({close, close + 1, close + len(newline)}):
        args = [formatter, "--style=" + style,
                "--assume-filename=" + filename,
                "--offset=" + str(start), "--length=" + str(end - start),
                "--cursor=" + str(close - 1)]
        result = subprocess.run(args + ["--output-replacements-xml"],
                                input=source, capture_output=True, check=True)
        root = ET.fromstring(result.stdout)
        assert root.attrib["incomplete_format"] == "false"
        edits = [(int(e.attrib["offset"]), int(e.attrib["length"]),
                  (e.text or "").encode()) for e in root.findall("replacement")]
        complete = source
        editor = source
        for offset, length, text in reversed(edits):
            complete = complete[:offset] + text + complete[offset + length:]
            inside = offset >= start and offset + length <= end
            removed = source[offset:offset + length]
            if b"{" in removed or b"}" in removed:
                assert inside, "brace deletion bundled with out-of-range whitespace"
            if inside:
                editor = editor[:offset] + text + editor[offset + length:]
        assert editor.count(b"{") == editor.count(b"}") == 1, editor
        following = next(line for line in editor.splitlines() if b"next();" in line)
        assert following == b"        next();", editor
        # Splitting XML edits must preserve the result and cursor calculation.
        plain = subprocess.run(args, input=source, capture_output=True, check=True).stdout
        cursor, formatted = plain.split(b"\n", 1)
        assert complete == formatted
        assert int(root.findtext("cursor")) == json.loads(cursor)["Cursor"]
