# <span style="color:#32d583">Lime</span>

<p>
  <img alt="CI" src="https://img.shields.io/github/actions/workflow/status/NuggiDev/Lime/ci.yml?branch=main&style=for-the-badge&label=CI&logo=github">
  <img alt="Language" src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus">
  <img alt="Build" src="https://img.shields.io/badge/build-CMake-064F8C?style=for-the-badge&logo=cmake">
  <img alt="License" src="https://img.shields.io/badge/license-MIT-32d583?style=for-the-badge">
</p>

<span style="color:#32d583">Lime</span> is a fast, zero-dependency C++ project scaffolder. It creates starter folders, source files, config files, and ignore files with one command:

```sh
Lime init [language] [path/to/folder/to/init/]
```

The goal is simple: pick a language, point Lime at a folder, and get a clean project layout that is ready for the normal tooling of that language.

## Features

- <span style="color:#32d583">One command</span> to initialize a project.
- <span style="color:#60a5fa">Native C++ CLI</span> built with CMake.
- <span style="color:#f59e0b">No runtime dependencies</span> for Lime itself.
- Refuses to overwrite generated files that already exist.
- Generates standard config files like `pom.xml`, `CMakeLists.txt`, `package.json`, `Cargo.toml`, `go.mod`, and `pyproject.toml`.
- Uses aliases like `cpp` / `c++`, `typescript` / `ts`, `javascript` / `js`, and `csharp` / `c#` / `cs`.

## Supported Languages

| Language | Aliases | Generated tooling |
| --- | --- | --- |
| Java | `java`, `maven` | Maven, `pom.xml`, `src/main/java` |
| C++ | `cpp`, `c++` | CMake, `src`, `include` |
| C | `c` | CMake, `src`, `include` |
| C# | `csharp`, `c#`, `cs` | SDK-style `.csproj`, .NET 8 |
| TypeScript | `typescript`, `ts` | npm, `package.json`, `tsconfig.json` |
| Ruby | `ruby`, `rb` | Bundler, Rake, gemspec, Minitest |
| Go | `go`, `golang` | Go modules with `go.mod` |
| JavaScript | `javascript`, `js`, `node` | npm and Node test runner |
| Rust | `rust`, `rs`, `cargo` | Cargo with `Cargo.toml` |
| Python | `python`, `py` | `pyproject.toml`, `src` layout |

## Build Lime

```sh
cmake -S . -B build
cmake --build build
```

On Windows with Visual Studio generators, the executable is usually created under a config folder:

```sh
build\Debug\Lime.exe --help
```

With single-config generators, it may be:

```sh
build\Lime --help
```

## Usage

Create a Java Maven project:

```sh
Lime init java apps/demo-java
```

Create a C++ CMake project:

```sh
Lime init cpp apps/demo-cpp
```

Create a Python project with `pyproject.toml`:

```sh
Lime init python tools/demo-python
```

Create a TypeScript project:

```sh
Lime init ts web/demo-ts
```

## Generated Examples

For `Lime init cpp apps/demo-cpp`, Lime creates:

```text
apps/demo-cpp/
  CMakeLists.txt
  README.md
  .gitignore
  include/demo_cpp/.gitkeep
  src/main.cpp
```

For `Lime init python tools/demo-python`, Lime creates:

```text
tools/demo-python/
  pyproject.toml
  README.md
  .gitignore
  src/demo_python/__init__.py
  src/demo_python/cli.py
  tests/test_smoke.py
```

## Safety

Lime creates the target directory if it does not exist. Before writing files, it checks every generated file path and stops if any file already exists. That keeps existing projects from being overwritten by accident.

## License

Lime is released under the MIT License. See [LICENSE](LICENSE).

