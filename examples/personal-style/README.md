# Personal clang-format style

This example uses the `ControlStatementBraces` extension from
[issue #2](https://github.com/galbraithjoseph/vds-format/issues/2), along with the
Allman placement and spacing from
[issue #1](https://github.com/galbraithjoseph/vds-format/issues/1).
The original request's "where" is interpreted as `while` for these C/C++ examples.
It does not change LLVM's repository-wide style.

## Run the example

Build `clang-format` from this fork; stock clang-format does not recognize the
new option. The reference output is checked with this fork's **24.0.0git** build.
Python 3 is needed only for the checker.

From the repository root, after configuring a build in `build`:

```sh
cmake --build build --target clang-format
build/bin/clang-format --style=file:examples/personal-style/.clang-format examples/personal-style/input.cpp
python examples/personal-style/check.py --clang-format build/bin/clang-format
```

On Windows, the executable is `build/bin/clang-format.exe`. The checker resolves
its fixtures relative to its own location, accepts an executable path containing
spaces, treats configuration warnings as errors, compares the actual formatted
output with `expected.cpp`, and verifies second-pass idempotence. Failures print
a diff or diagnostic and return status 1. Line endings are normalized for comparison.

To apply the sample elsewhere, copy `.clang-format` into the project's root or
pass `--style=file:<path>`. It uses tabs with a width of four, left-aligned
pointers and references, and indented namespace contents.
Nonempty braced initializers use interior spaces, as in `int x{ 0 };`
and `int values[]{ 1, 2, 3 };`, via `Cpp11BracedListStyle: Block`. Empty
initializers remain `{}`. Wrapped initializer lists use block indentation and
put the closing brace on its own line. Empty blocks may stay
on one line; function bodies are expanded. Short lambdas may stay on one line
within the 120-column limit. Multiline lambda braces align with the enclosing
statement. Function calls keep arguments together when they fit; wrapped calls
use one argument per line, aligned with the first argument beside the function
name when it fits there. Function declarations and definitions likewise keep
parameters together when they fit and use one parameter per line when wrapped.
Constructor
initializer lists keep the colon on the signature line and put each initializer
on its own line, indented four columns. Function definitions put return types
on their own line; declarations keep the return type and function name together
when they fit within the column limit. Template declarations use
`template<class X>` spacing and always break before the following declaration.
Up to two consecutive blank lines are
preserved. Other unspecified settings inherit LLVM defaults.
`input.cpp` and `expected.cpp` are formatting fixtures with external
function declarations, not complete programs.

## Brace policy

```yaml
ControlStatementBraces:
  Mode: SingleStatement
  IfChain: Consistent
  WrappedCondition: Require
BreakBeforeBraces: Allman
```

- `simple` removes optional braces from both single-statement branches.
- `mixedBranches` and `chained` brace every branch because one branch contains
  two statements. An else-if stays an else-if.
- `wrappedCondition` forces a condition to wrap at `ColumnLimit: 120` and braces
  both single-statement branches.
- `polling` retains its two-statement while body; `counting` removes the optional
  braces around its single statement.

Brace placement remains Allman. Conditions are checked after formatting a
canonical braced representation, including if initializers. Input line breaks
alone do not trigger braces, and a single body statement spanning several lines
is still one statement. With attached braces, a brace that forces condition
wrapping is retained, avoiding oscillation between invocations.

The extension supports C, C++, and Objective-C `if`, `for`, `while`, and `do`;
other languages are unaffected. `Mode: Leave` is the default and preserves brace
presence. `Mode: Always` inserts braces around nonempty bodies. `IfChain` and
`WrappedCondition` apply only to `SingleStatement`; their alternatives are
`Independent` and `Ignore`. An active policy cannot be combined with
`InsertBraces: true` or `RemoveBracesLLVM: true`.

Empty compound bodies and null-statement bodies remain unchanged. Comments,
attributes, preprocessor boundaries, disabled formatting, and possible dangling
else binding can prevent transformations. Review brace changes: the formatter
does not have complete semantic information.

For selection and paste formatting, brace presence is preserved if the policy
would change braces outside the requested ranges. Boundary whitespace edits
do not prevent removal when both braces are inside the range. Format Document applies
the policy to the complete file; a range containing both brace edits can also
remove them safely.

## Function separators — [issue #4](https://github.com/galbraithjoseph/vds-format/issues/4)

`FunctionDeclarationSeparatorColumn: 80` adds a standalone separator before
function declarations and definitions, above attached documentation. Its first
slash aligns with the first line of the declaration (including template and
attribute prefixes), and its final tilde ends at display column 80. The slashes
count toward that width: zero indentation uses 78 tildes; four columns of
indentation use 74. Tabs use `TabWidth` display columns. The fixture checker
checks exact output and second-pass idempotence for both `input.cpp` and the
conditional-branch regression fixture `preprocessor-input.cpp`.

The option defaults to zero (disabled). Existing standalone `//` plus tildes
lines are normalized; existing duplicate separators and ordinary comments are
preserved. Conditional preprocessor branches do not cause repeated insertion
for declarations following the conditional. If indentation leaves
less than three columns, insertion and resizing are skipped. Preprocessor
and formatting-disable boundaries are preserved. Partial formatting changes a
separator only when its position, attached comments, and the complete declaration
header are in one requested range. Inline declarations sharing a line with
preceding code are skipped. Like other declaration-aware clang-format options,
recognition uses token annotations rather than semantic analysis.

The previous stock-only baseline from #1 was verified with clang-format 17.0.6
and 23.1.3. Its `RemoveBracesLLVM` approximation left single-statement branches
unbraced beside multi-statement branches and did not add braces for wrapped
conditions. The current fixtures exercise the implemented policy instead.

## Selective parenthesis spacing (issue #3)

With `SpacesInParens: Custom`, `ExceptForLoops: true` overrides
`InConditionalStatements` and `Other` for traditional and range-based for
headers. The sample now produces `for (int index = 0; index < count; ++index)`
alongside spaced if/while conditions.

`InNamedCalls: [ASSERT, VERIFY]` produces `ASSERT( expression );` and
`VERIFY( expression );` while ordinary calls
remain `x(y)`. Names match the final identifier exactly and case sensitively,
including qualified/member calls. Declarations and preprocessor directives are
excluded. Empty calls follow `InEmptyParentheses`; `ExceptDoubleParentheses`
takes precedence. Spacing before parentheses is unchanged. Both options are
opt-in and ignored unless `SpacesInParens: Custom` is enabled. Other control
statements retain their existing conditional spacing; the for exception also
applies to for headers in other supported languages.

`ExceptSwitchStatements: true` similarly produces `switch (x)` while keeping
spaces inside if/while conditions. It overrides `InConditionalStatements` and
`Other`, defaults to false, and leaves the space before `(` unchanged. The
`switchSpacing` fixture demonstrates this alongside `ASSERT` and `VERIFY`.
