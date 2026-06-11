# Packlang — Implementation Plan

Following Ghuloum's Incremental Approach to Compiler Construction.
Each step adds one feature. The compiler always works. No broken compilers.

---

## Phase 0: Project Scaffolding

Replace the C++/CMake skeleton with a minimal C bootstrap project.

| Action | What |
|---|---|
| Delete | `CMakeLists.txt`, `src/`, `tests/`, `.clang-tidy` |
| Update | `.gitignore` — add `packc` binary |
| Create | `Makefile` — 3 lines: build `packc.c` with `cc` |
| Create | `packc.c` — single C file, the entire compiler |
| Create | `examples/` — test `.pack` files |
| Update | `.github/workflows/ci.yml` — `make && make test` |
| Update | `.opencode/AGENTS.md` — reference C + Makefile |

---

## Phase 1: Bootstrap Compiler (Pack → C)

The bootstrap compiler reads `.pack` files and emits equivalent C code.
Each step ends with a working compiler and a `packagent` commit.

### Step 1 — Lexer
- Read a `.pack` file into memory
- Tokenize: identifiers, numbers, operators, symbols
- Print tokens to stdout (debug mode)
- Output: `./packc hello.pack` → list of tokens

### Step 2 — Parser (expressions)
- Parse integer literals → AST nodes
- Parse binary ops (+, -, *, /) → `BinOp` nodes
- Print AST (debug mode)

### Step 3 — C Codegen (expressions)
- Walk AST, emit equivalent C code
- Expression evaluates to `int`, returned from `main()`
- Emitted C compiles with `gcc`/`clang` and runs
- `2 + 3` → C: `int main() { return 2 + 3; }` → exit code 5

### Step 4 — Variables
- `let x = 5`
- C variable declaration in emitted code
- Variable references in expressions

### Step 5 — Conditionals
- `<`, `>`, `==`, `!=` comparisons
- `if` / `else` control flow
- C emit: `if (...) { ... } else { ... }`

### Step 6 — Loops
- `while cond { body }`
- C: `while (...) { ... }`

### Step 7 — Blocks and scoping
- `{ let x = 1; x }` — inner scope
- Variable shadowing

### Step 8 — Functions
- `fn name(params) { body }`
- Parameters, return values, function calls
- C: emit as C functions

### Step 9 — Type annotations
- `let x: int = 5`
- `fn add(a: int, b: int) -> int { ... }`
- Compile-time type checking

### Step 10 — Strings
- `let s: str = "hello"`
- C: `char*` with runtime support
- String ops: concat, length

### Step 11 — Arrays
- `let arr: [int] = [1, 2, 3]`
- Indexing: `arr[0]`
- Bounds checking (compile-time where possible)

### Step 12 — Structs
- `struct Point { x: int, y: int }`
- Field access: `p.x`
- C: emit as C struct

### Step 13 — Enums and pattern matching
- `enum Option<T> { Some(T), None }`
- `match val { Some(x) => ..., None => ... }`
- Exhaustiveness checking

---

## Phase 2: Memory Safety

### Step 14 — Ownership (move semantics)
- Single owner per value
- Assignment moves ownership
- Compile-time: use of moved value → error
- C emit: raw pointers, no runtime cost

### Step 15 — Borrowing
- `&x` (shared reference, multiple readers)
- `&mut x` (exclusive reference, single writer)
- Compile-time: XOR rule enforced

### Step 16 — Lifetime inference
- Function signatures: lifetimes inferred
- Minimal annotations needed
- C emit: compile-time check only

---

## Phase 3: Error Handling

### Step 17 — Result and Option types
- `Result<T, E>`, `Option<T>` as builtins
- `?` operator for error propagation
- `unwrap()` for panics

---

## Phase 4: Generics

### Step 18 — Generic functions
- `fn max<T: Ord>(a: T, b: T) -> T`
- Monomorphization: separate C function per type

---

## Phase 5: Modules

### Step 19 — Module system
- `import "file.pack"` — separate files
- `pub fn ...` — public API
- C: `#include` or concatenated compilation

---

## Phase 6: Standard Library

### Step 20 — Stdlib in Pack
- `pack::io::print`, `pack::str::String`, `pack::vec::Vec`
- Written in Pack, compiled by Pack compiler

---

## Phase 7: Self-Hosting

### Step 21 — Rewrite compiler in Pack
- `packc.c` rewritten as `packc.pack`
- Lexer, parser, codegen in Pack
- Stage 0 (C) compiles Stage 1 (Pack)

### Step 22 — Bootstrap verification
- Stage 1 compiles itself
- Output matches Stage 0 output for all tests
- Compiler is self-hosting

---

## Phase 8: LLVM Backend

### Step 23 — LLVM IR emission
- Optional: `--backend=llvm`
- Enables LLVM optimization pipeline
- Both C and LLVM backends coexist

---

## Phase 9: Agent Primitives

### Step 24 — Pipeline operator
- `tool1 |> tool2 |> tool3`
- Desugars to chained function calls, type-checked

### Step 25 — Structured concurrency
- `parallel { let a = f(); let b = g() }`
- C: pthreads or platform threads
- Ownership prevents data races

### Step 26 — Tool composition
- Typed tools, compile-time composition checking
- MCP integration via stdlib
