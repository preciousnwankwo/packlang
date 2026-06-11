# Pack Compiler - Session State

## Goal
Build a systems programming language called Pack (`.pack` extension) using Ghuloum's incremental compiler construction approach, bootstrapped from C with self-hosting as the end goal.

## Completed Steps
- **Step 1**: Lexer — tokenizes `.pack` files, supports `//` and `/* */` comments, numbers, identifiers, strings, all operators, `--tokens` flag
- **Step 2**: Parser — recursive descent, AST printing, correct operator precedence (`*`/`/` > `+`/`-`), unary minus, parenthesized grouping, left associativity
- **Step 3**: C Codegen — emits compilable C from expression AST, works with `gcc`/`clang`, `--emit-c` flag
- **Step 4**: Variables — `let x = 5;` with `let` keyword, variable references in expressions, program/statement AST, multiple variable support
- **Step 5**: Conditionals — `if`/`else` expressions with comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`), blocks via `NODE_BLOCK`, GCC statement expression codegen
- **Step 6**: `while` loops — `while cond { body }` expressions, `NODE_WHILE`, `codegen_block_all_stmts` helper for side-effect emission, optional semicolons after while blocks
- **Step 7**: Blocks and lexical scoping — standalone `{ let x = 5; x + 1 }` block expressions, `SymTable` with push/pop scopes, duplicate `let` detection, undeclared variable detection, variable shadowing, scoped C codegen using `({...})`

## Next Steps (per PLAN.md)
- **Step 8**: Functions with parameters and return values
- **Step 9**: Type checking / type annotations
- **Step 10**: Arrays and indexing

## Key Design Decisions
- Rust-style `let` for variable bindings
- `if`/`else` as expressions (Rust-style), not statements
- `--tokens` / `--emit-c` flags for debug; default mode prints AST
- `-std=c99` for maximum CI compat
- C codegen uses GCC statement expressions `({ ... })` for if-expression value capture
- Comparison operators are left-associative (parsed as `parse_comparison` between `parse_expr` and `parse_term`)

## Constraints
- Compiler is a single C file (`packc.c`) — stays one file until concrete reason to split
- No LLVM dependency; C backend emits C code that compiles with any C89+ compiler
- Ghuloum principle: every commit produces a working compiler

## Files
- `packc.c`: Single-file compiler — lexer, parser, AST, codegen all in one file
- `Makefile`: Builds `packc`, test target emits+compiles+runs `.pack` files
- `opencode.jsonc`: MCP server config (GitHub), agent instructions reference
- `.opencode/AGENTS.md`: Agent instructions for skills, build commands, Ghuloum approach
- `.github/workflows/ci.yml`: CI pipeline — `make && make test`
- `PLAN.md`: Full 26-step incremental compiler construction plan
- `examples/hello.pack`: Test file (conditionals example)
- `examples/conditional.pack`: Test file (if/else with let)
- `examples/while.pack`: Test file (while loop)
- `examples/scope.pack`: Test file (block scope / shadowing)
