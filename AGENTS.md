# AGENTS.md

## Project

linyaps-box is a C++17 OCI container runtime (`ll-box`) for the [linyaps](https://github.com/OpenAtom-Linyaps/linyaps) Linux desktop app distribution toolkit. It uses Linux namespaces, cgroups, and bind mounts to sandbox desktop applications.

## Build

The project uses CMake with presets. The one-command dev build:

```bash
cmake --workflow --preset=dev
```

This configures into `build-dev/`, builds with ASan/UBSan and `-Werror`, then runs all tests. Other presets: `release` (`build-release/`), `static` (`build-static/`), `ci` (`build-ci/`).

Step-by-step equivalent:

```bash
cmake --preset=dev
cmake --build --preset=dev
ctest --preset=dev
```

Dependencies (`nlohmann_json`, `CLI11`) are vendored in `external/` and used as fallback if system packages are not found. The dev preset enables CPM to download them automatically.

## Tests

- **Unit tests** (`tests/ll-box-ut/`): GTest-based, always enabled in top-level builds. Binary: `ll-box-ut`.
- **Smoke tests** (`tests/ll-box-st/`): JSON-driven integration tests that run the `ll-box` binary against a real container image. Enabled only with `-Dlinyaps-box_ENABLE_SMOKE_TESTS=ON` (on by default in dev/ci presets). Require `jq`, `umoci`, and `podman` on the host. The test runner pulls `docker.io/comixhe1895/linyaps-box-st:stable-slim`.

Run a single unit test (after build):

```bash
ctest --test-dir build-dev -R <test_name>
```

Run a single smoke test:

```bash
./tests/ll-box-st/ll-box-st ./build-dev/ll-box ./build-dev/tests/ll-box-st/st-data ./tests/ll-box-st/01-run-whoami.json
```

Smoke tests use LSAN suppressions for known glibc leaks (`tests/ll-box-st/glibc_leaks.txt`).

## Code layout

- `app/ll-box/src/main.cpp` - entry point, delegates to `linyaps_box::main()`
- `src/linyaps_box/` - static library with all runtime logic
  - `command/` - CLI subcommands (run, exec, kill, list)
  - `container.cpp` - core container lifecycle
  - `runtime.cpp` - OCI runtime implementation
  - `config.cpp` - OCI config parsing (nlohmann_json)
  - `utils/` - Linux-specific utilities (cgroups, namespaces, mounts, signals, etc.)
- `tests/ll-box-ut/` - unit tests
- `tests/ll-box-st/` - smoke tests (JSON configs + bash runner)
- `external/` - vendored dependencies (formatting disabled here)
- `cmake.external/` - custom Find modules for vendored deps

## Code style

- C++ formatting: clang-format with WebKit-based style, 100 column limit. Config in `.clang-format`.
- CMake formatting: cmake-format with custom CPM command parsers. Config in `.cmake-format.py`.
- Format all source files: `./tools/format.sh` (or `./tools/format.sh clang-format-17` for a specific version).
- `external/` has formatting disabled via its own `.clang-format` and `.cmake-format.py`.
- Dev and CI presets compile with `-Wall -Wextra -Wpedantic -Werror` -- all warnings are errors.

## Conventions

- License headers are required on all source files (REUSE/SPDX). Use `LGPL-3.0-or-later` for project code:
  ```
  // SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
  //
  // SPDX-License-Identifier: LGPL-3.0-or-later
  ```
- CI runs commitlint -- commit messages must follow [Conventional Commits](https://www.conventionalcommits.org/).
- New source files must be manually added to the `linyaps-box_LIBRARY_SOURCE` list in the root `CMakeLists.txt`.
- `version.h` is generated from `version.h.in` at configure time; do not edit it directly.
- The `stdc++fs` library is linked for GCC 8 compatibility (filesystem support).

## Build options reference

| Option | Default | Notes |
|--------|---------|-------|
| `linyaps-box_ENABLE_SECCOMP` | OFF | Requires libseccomp >= 2.3.3 |
| `linyaps-box_ENABLE_CAP` | ON | Requires libcap >= 2.25 |
| `linyaps-box_ENABLE_SMOKE_TESTS` | OFF | Needs jq, umoci, podman |
| `linyaps-box_ENABLE_COVERAGE` | OFF | Requires smoke or unit tests enabled |
| `linyaps-box_STATIC` | OFF | Produces fully static binary |
| `linyaps-box_ENABLE_CPM` | OFF | Auto-download deps via CPM.cmake |
