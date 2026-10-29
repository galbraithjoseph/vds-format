# Personal clang-format style: stock baseline

This example implements the stock-compatible part of
[issue #1](https://github.com/galbraithjoseph/vds-format/issues/1). It does not
change LLVM's repository-wide style or implement the custom formatter extensions.
The original request's "where" is interpreted as `while` for these C/C++ examples.

## Run the example

Use stock clang-format **17 or newer**. The reference output was generated and
checked with **clang-format 17.0.6**, distributed by the `clang-format` Python
package. LLVM defaults can change between versions; use 17.0.6 when reproducing
this fixture exactly. The same checks also pass with stock **23.1.3**, producing
identical output. Python 3 is needed only for the validation script.

From the repository root:

```sh
clang-format --version
clang-format --style=file:examples/personal-style/.clang-format examples/personal-style/input.cpp
python examples/personal-style/check.py
```

The formatter prints the result without modifying `input.cpp`. To select a
versioned executable or an executable whose path contains spaces:

```sh
python examples/personal-style/check.py --clang-format "/path/to/clang-format-17"
```

On Windows, `py -3` can replace `python`. The checker resolves the configuration
and fixtures relative to its own location, so it also works from other working
directories. It checks configuration parsing with warnings treated as errors,
compares the formatted input with `expected.cpp`, and checks that a second pass
does not change the result. Failures produce a diff or diagnostic and exit with
status 1. Line endings are normalized when reading and comparing files.

To apply the sample to another project, copy `.clang-format` into that project's
root or pass its path explicitly with `--style=file:<path>`. It inherits LLVM
defaults, including two-space indentation. Do not copy `expected.cpp` into a
production build; it is a formatting fixture with declarations for external
functions, not a complete program.

## What the files demonstrate

- `.clang-format`: stock settings only; no fork-specific options.
- `input.cpp`: deliberately unformatted declarations and definitions, simple
  if/else, multiple statements, a condition that wraps at the 80-column limit,
  while, a traditional for loop, and unary logical not.
- `expected.cpp`: actual output from stock clang-format 17.0.6, including its
  limitations. This is the checker's reference, not a mock-up of future behavior.

Retained function and control-block braces use Allman (vertical) placement.
Control statements have a space before `(`; conditional parentheses have inner
spaces, as in `if ( ready )` and `while ( ! bStopped )`. `SpaceAfterLogicalNot`
adds the space after `!`. Short functions and control bodies remain multiline.

`RemoveBracesLLVM: true` approximates the desired optional-brace rule using
LLVM's existing policy. `InsertBraces: false` avoids unconditional insertion.
**Review brace-removal changes:** upstream cautions that clang-format lacks
complete semantic information and this option can produce incorrect formatting.

## Remaining differences

### Brace policy — [issue #2](https://github.com/galbraithjoseph/vds-format/issues/2)

The stock policy does not implement the requested combination of statement
counts, wrapped conditions, and consistent braces across an entire if chain.
In `expected.cpp`, `mixedBranches` keeps its two-statement branch braced but
leaves the single-statement else unbraced. The desired result would be:

```cpp
if ( ready )
{
  prepare();
  run();
}
else
{
  wait();
}
```

Likewise, `wrappedCondition` has no braces around its single-statement bodies
even though its condition wraps. The requested behavior would brace both bodies.
The sample does not claim to enforce either rule.

### For spacing — [issue #3](https://github.com/galbraithjoseph/vds-format/issues/3)

Stock `SpacesInParensOptions.InConditionalStatements` groups `for` with `if`
and `while`. The actual output is:

```cpp
for ( int index = 0; index < count; ++index )
  step(index);
```

The requested output instead has no spaces immediately inside the for header:

```cpp
for (int index = 0; index < count; ++index)
  step(index);
```

Setting `InConditionalStatements: false` fixes for spacing but also removes
the desired spaces inside if/while parentheses. It cannot express both rules.

### Function separators — [issue #4](https://github.com/galbraithjoseph/vds-format/issues/4)

The declarations and definitions in `expected.cpp` have no generated tilde line.
Stock clang-format cannot insert a `//` followed by tildes before every function
declaration, align its first slash with the declaration, and resize it so the
final tilde is in one-based column 80. `ColumnLimit: 80` controls code wrapping;
it does not synthesize such comments.

See the upstream [style option reference](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
for `BreakBeforeBraces`, `SpacesInParensOptions`, `SpaceAfterLogicalNot`,
`RemoveBracesLLVM`, and `InsertBraces`.
