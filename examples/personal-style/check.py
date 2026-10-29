#!/usr/bin/env python3
"""Check the personal-style formatting fixture without modifying any files."""

import argparse
import difflib
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang-format", default="clang-format",
                        help="clang-format executable built from this fork")
    args = parser.parse_args()
    directory = Path(__file__).resolve().parent

    def invoke(options, source=None):
        result = subprocess.run(
            [args.clang_format, *options], input=source, text=True,
            encoding="utf-8", stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            check=True,
        )
        if result.stderr:
            print(result.stderr, file=sys.stderr, end="")
        return result.stdout

    try:
        print(invoke(["--version"]).strip())
        options = ["--Werror", "--style=file:" + str(directory / ".clang-format"),
                   "--assume-filename=sample.cpp"]
        # Parsing must succeed without unknown-option warnings.
        invoke([*options, "--dump-config"])
        source = (directory / "input.cpp").read_text(encoding="utf-8")
        expected = (directory / "expected.cpp").read_text(encoding="utf-8")
        actual = invoke(options, source)
        second_pass = invoke(options, actual)
        passed = True
        for label, before, after in [
            ("Expected output", expected, actual),
            ("Second-pass idempotence", actual, second_pass),
        ]:
            if before != after:
                passed = False
                print(label + ": FAIL", file=sys.stderr)
                sys.stderr.writelines(difflib.unified_diff(
                    before.splitlines(keepends=True),
                    after.splitlines(keepends=True),
                    fromfile=label + " (expected)", tofile=label + " (actual)",
                ))
        if not passed:
            return 1
        print("PASS: configuration parses, expected output matches, "
              "and second pass is unchanged")
        return 0
    except (OSError, subprocess.CalledProcessError) as error:
        print(str(error), file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError) and error.stderr:
            print(error.stderr, file=sys.stderr, end="")
        return 1


if __name__ == "__main__":
    sys.exit(main())
