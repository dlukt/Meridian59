# Repository Guidelines

## Project Structure & Module Organization
Meridian 59 is a multi-module C/C++ codebase with platform-specific build paths.
- `blakserv/`: game server sources and Linux makefile (`makefile.linux`).
- `clientd3d/`: Windows Direct3D client.
- `blakcomp/` and `kod/`: Blakod compiler and game logic sources.
- `util/`, `bbgun/`, `makebgf/`, `resource/`, `module/`: shared tools and content pipelines.
- `tests/`: lightweight unit test harness for shared/server utilities.
- `run/`: generated runtime install trees (`run/server`, `run/localclient`), plus scripts in `run/scripts/`.

## Build, Test, and Development Commands
- Windows debug build (from repo root): `nmake`
  - Builds server, client, modules, tools, and resources.
- Windows release build: `nmake RELEASE=1`
- Linux/macOS server build: `cd blakserv && make -f makefile.linux`
- Linux/macOS compiler + kod build:
  - `cd blakcomp && make -f makefile.linux`
  - `cd ../kod && make -f makefile.linux`
- Run unit tests: `make -C tests test`
- Optional spellcheck parity with CI:
  - `codespell --ignore-regex '[~`][a-zA-Z]+' --ignore-words .github/.codespellignore dlg/npc.tab kod`

## Coding Style & Naming Conventions
- Formatting is enforced via `.clang-format`:
  - 3-space indentation, no tabs, max line length 120, wrapped braces (Allman-like).
- C/C++ uses C++20 toolchains (`-std=c++20` in Linux builds).
- Follow existing filename patterns:
  - modules/utilities use lowercase names (e.g., `json_utils.c`),
  - tests use `test_*.cpp`/`test_*.h`.
- Prefer small, focused functions and keep warnings clean (`-Werror` is used on Linux).

## Testing Guidelines
- Test framework lives in `tests/test_framework.h` (assert macros + runner).
- Add new tests in `tests/test_*.cpp` and register them in `tests/test_main.cpp`.
- Keep tests deterministic and self-contained (use temp files and cleanup patterns already present).
- Run `make -C tests test` before opening a PR.

## Commit & Pull Request Guidelines
- Use short, imperative commit subjects; existing history favors prefixes like `fix:` and `perf:`.
- Keep each commit logically scoped (bugfix, refactor, test update, etc.).
- PRs should include:
  - what changed and why,
  - linked issue/PR references when applicable (e.g., `#1351`),
  - test evidence (unit test output, platform build notes),
  - screenshots only for UI/editor changes (e.g., `roomedit`/client visuals).
