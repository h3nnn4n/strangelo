# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build/Run Commands

- Configure: `cmake -B build`
- Build: `cmake --build build`
- Run: `cmake --build build --target run`
- Debug with gdb: `cmake --build build --target gdb`
- Clean: `rm -rf build`
- Debug build: `cmake -DCMAKE_BUILD_TYPE=Debug -B build`
- Release build: `cmake -DCMAKE_BUILD_TYPE=Release -B build`
- Profiling: `cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build`

## Code Style Guidelines
- C99 for C code, C++11 for C++ code
- 4-space indentation (no tabs)
- Use MIT license header for all source files
- Include guards for header files
- Functions use snake_case naming
- Braces on same line for functions and control structures
- Standard includes first, then 3rd party libs, then local includes
- Local includes use quotes, system/library includes use angle brackets
- Error handling: prefer early returns, use assert where appropriate
- Linting follows cpplint rules defined in CPPLINT.cfg
- Max line length is not enforced (filtered in CPPLINT.cfg)
- Use consistent spacing around operators