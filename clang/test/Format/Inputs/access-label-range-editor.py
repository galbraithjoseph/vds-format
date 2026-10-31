"""Editors must be able to apply access-label indentation within their range."""
import json
import subprocess
import sys
import xml.etree.ElementTree as ET

formatter, filename = sys.argv[1:]
style = json.dumps({"BasedOnStyle": "LLVM", "BreakBeforeBraces": "Allman",
                    "IndentWidth": 4, "TabWidth": 4, "UseTab": "Always",
                    "AccessModifierOffset": -4, "IndentAccessModifiers": False})
for newline in [b"\n", b"\r\n"]:
    for access in [b"private", b"protected", b"public"]:
        source = newline.join([b"class Foo", b"{", b"", b"\t" + access + b":", b"};", b""])
        name = source.index(access)
        for start in [name - 1, name - 2]:
            end = name + len(access) + 1
            args = [formatter, "--style=" + style, "--assume-filename=" + filename,
                    "--offset=" + str(start), "--length=" + str(end - start),
                    "--output-replacements-xml"]
            root = ET.fromstring(subprocess.check_output(args, input=source))
            editor = source
            complete = source
            for edit in reversed(root.findall("replacement")):
                offset, length = int(edit.attrib["offset"]), int(edit.attrib["length"])
                text = (edit.text or "").encode()
                complete = complete[:offset] + text + complete[offset + length:]
                if offset >= start and offset + length <= end:
                    editor = editor[:offset] + text + editor[offset + length:]
            assert newline + access + b":" in editor, editor
            assert editor == complete, (editor, complete)
print("PASS: range-filtered access-label indentation for all access levels and LF/CRLF.")
