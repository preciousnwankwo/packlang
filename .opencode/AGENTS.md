# Packlang Agent Instructions

You are building a programming language called **packlang** in C++.

## Skills

Always load and use skills from `.agents/skills/` when relevant. The available skills cover:
- **Build systems**: cmake, make, ninja, meson, bazel
- **Compilers**: gcc, clang, llvm, cross-gcc, pgo, cpp-modules, cpp-templates
- **Debugging**: gdb, lldb, core-dumps, debug-optimized-builds, concurrency-debugging
- **Profiling**: valgrind, linux-perf, flamegraphs, heaptrack, hardware-counters
- **Low-level**: interpreters, simd-intrinsics, cpu-cache-opt, assembly (x86/ARM/RISC-V)
- **Runtimes**: sanitizers, fuzzing, binary-hardening, dynamic-linking, linkers-lto

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## Standards

- C++23, Clang/GCC, latest standards
- Follow the skill guidance for each subdomain
