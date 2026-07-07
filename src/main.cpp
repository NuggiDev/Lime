#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

struct FileSpec {
    fs::path path;
    std::string content;
};

using Files = std::vector<FileSpec>;
using TemplateFn = Files (*)(const std::string&);

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string replace_all(std::string text, std::string_view from, std::string_view to) {
    std::size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
    return text;
}

std::string render(std::string text, const std::vector<std::pair<std::string_view, std::string>>& values) {
    for (const auto& [key, value] : values) {
        text = replace_all(std::move(text), key, value);
    }
    return text;
}

std::string project_name_from_path(const fs::path& path) {
    fs::path normalized = path.lexically_normal();
    std::string name = normalized.filename().string();
    if (name.empty() || name == "." || name == "..") {
        name = "my-project";
    }

    for (char& c : name) {
        const bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
        if (!ok) {
            c = '-';
        }
    }
    return name;
}

std::string package_name(std::string name) {
    for (char& c : name) {
        const bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
        c = ok ? static_cast<char>(std::tolower(static_cast<unsigned char>(c))) : '-';
    }
    if (name.empty() || name.front() == '-' || name.front() == '_') {
        name = "my-project";
    }
    return name;
}

std::string identifier(std::string name) {
    for (char& c : name) {
        const bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '_';
        if (!ok) {
            c = '_';
        }
    }
    if (name.empty() || std::isdigit(static_cast<unsigned char>(name.front()))) {
        name = "project_" + name;
    }
    return name;
}

std::string python_module(std::string name) {
    return identifier(replace_all(std::move(name), "-", "_"));
}

std::string java_package(const std::string& name) {
    return "com.example." + lower(identifier(name));
}

std::string java_package_path(const std::string& package) {
    return replace_all(package, ".", "/");
}

std::string ruby_module(std::string name) {
    std::string result;
    bool capitalize_next = true;
    for (char c : name) {
        if (c == '-' || c == '_') {
            capitalize_next = true;
            continue;
        }
        result += capitalize_next ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : c;
        capitalize_next = false;
    }
    return result.empty() ? "App" : result;
}

Files java_files(const std::string& name) {
    const std::string package = java_package(name);
    const std::string package_path = java_package_path(package);
    return {
        {"pom.xml", render(R"(<project xmlns="http://maven.apache.org/POM/4.0.0"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
  <groupId>com.example</groupId>
  <artifactId>{{package_name}}</artifactId>
  <version>0.1.0-SNAPSHOT</version>
  <properties>
    <maven.compiler.release>21</maven.compiler.release>
    <project.build.sourceEncoding>UTF-8</project.build.sourceEncoding>
  </properties>
</project>
)", {{"{{package_name}}", package_name(name)}})},
        {fs::path("src/main/java") / package_path / "App.java", render(R"(package {{package}};

public final class App {
    private App() {
    }

    public static void main(String[] args) {
        System.out.println("Hello from {{name}}!");
    }
}
)", {{"{{package}}", package}, {"{{name}}", name}})},
        {".gitignore", "target/\n.idea/\n*.iml\n"},
        {"README.md", "# " + name + "\n\nMaven Java starter project.\n\n```sh\nmvn test\n```\n"},
    };
}

Files cpp_files(const std::string& name) {
    const std::string target = identifier(name);
    return {
        {"CMakeLists.txt", render(R"(cmake_minimum_required(VERSION 3.20)
project({{target}} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable({{target}} src/main.cpp)
target_include_directories({{target}} PRIVATE include)
)", {{"{{target}}", target}})},
        {"src/main.cpp", render(R"(#include <iostream>

int main() {
    std::cout << "Hello from {{name}}!\n";
    return 0;
}
)", {{"{{name}}", name}})},
        {fs::path("include") / target / ".gitkeep", ""},
        {".gitignore", "build/\nout/\n*.user\n.vscode/\n"},
        {"README.md", "# " + name + "\n\nCMake C++ starter project.\n\n```sh\ncmake -S . -B build\ncmake --build build\n```\n"},
    };
}

Files c_files(const std::string& name) {
    const std::string target = identifier(name);
    return {
        {"CMakeLists.txt", render(R"(cmake_minimum_required(VERSION 3.20)
project({{target}} VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable({{target}} src/main.c)
target_include_directories({{target}} PRIVATE include)
)", {{"{{target}}", target}})},
        {"src/main.c", render(R"(#include <stdio.h>

int main(void) {
    puts("Hello from {{name}}!");
    return 0;
}
)", {{"{{name}}", name}})},
        {fs::path("include") / target / ".gitkeep", ""},
        {".gitignore", "build/\nout/\n.vscode/\n"},
        {"README.md", "# " + name + "\n\nCMake C starter project.\n\n```sh\ncmake -S . -B build\ncmake --build build\n```\n"},
    };
}

Files csharp_files(const std::string& name) {
    const std::string safe = identifier(name);
    return {
        {safe + ".csproj", render(R"(<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net8.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
    <RootNamespace>{{safe}}</RootNamespace>
  </PropertyGroup>
</Project>
)", {{"{{safe}}", safe}})},
        {"Program.cs", "Console.WriteLine(\"Hello from " + name + "!\");\n"},
        {".gitignore", "bin/\nobj/\n.vs/\n.vscode/\n"},
        {"README.md", "# " + name + "\n\n.NET C# starter project.\n\n```sh\ndotnet run\n```\n"},
    };
}

Files typescript_files(const std::string& name) {
    const std::string package = package_name(name);
    return {
        {"package.json", render(R"({
  "name": "{{package}}",
  "version": "0.1.0",
  "type": "module",
  "private": true,
  "scripts": {
    "build": "tsc",
    "dev": "tsx src/index.ts",
    "start": "node dist/index.js"
  },
  "devDependencies": {
    "@types/node": "^20.0.0",
    "tsx": "^4.0.0",
    "typescript": "^5.0.0"
  }
}
)", {{"{{package}}", package}})},
        {"tsconfig.json", R"({
  "compilerOptions": {
    "target": "ES2022",
    "module": "NodeNext",
    "moduleResolution": "NodeNext",
    "strict": true,
    "outDir": "dist",
    "rootDir": "src",
    "sourceMap": true
  },
  "include": ["src"]
}
)"},
        {"src/index.ts", "console.log(\"Hello from " + name + "!\");\n"},
        {".gitignore", "node_modules/\ndist/\n.env\n"},
        {"README.md", "# " + name + "\n\nTypeScript starter project.\n\n```sh\nnpm install\nnpm run dev\n```\n"},
    };
}

Files javascript_files(const std::string& name) {
    const std::string package = package_name(name);
    return {
        {"package.json", render(R"({
  "name": "{{package}}",
  "version": "0.1.0",
  "type": "module",
  "private": true,
  "scripts": {
    "start": "node src/index.js",
    "test": "node --test"
  }
}
)", {{"{{package}}", package}})},
        {"src/index.js", "console.log(\"Hello from " + name + "!\");\n"},
        {"test/index.test.js", R"(import test from "node:test";
import assert from "node:assert/strict";

test("starter test", () => {
  assert.equal(1 + 1, 2);
});
)"},
        {".gitignore", "node_modules/\n.env\ncoverage/\n"},
        {"README.md", "# " + name + "\n\nJavaScript starter project.\n\n```sh\nnpm start\nnpm test\n```\n"},
    };
}

Files ruby_files(const std::string& name) {
    const std::string lib = python_module(name);
    const std::string module = ruby_module(name);
    return {
        {"Gemfile", R"(source "https://rubygems.org"

gem "rake", "~> 13.0"
gem "minitest", "~> 5.0"
)"},
        {lib + ".gemspec", render(R"(Gem::Specification.new do |spec|
  spec.name = "{{package}}"
  spec.version = "0.1.0"
  spec.summary = "Starter Ruby project"
  spec.authors = ["Your Name"]
  spec.files = Dir["lib/**/*.rb", "test/**/*.rb", "README.md"]
  spec.required_ruby_version = ">= 3.1"
end
)", {{"{{package}}", package_name(name)}})},
        {"Rakefile", R"(require "rake/testtask"

Rake::TestTask.new do |task|
  task.libs << "test"
  task.test_files = FileList["test/**/*_test.rb"]
end

task default: :test
)"},
        {fs::path("lib") / (lib + ".rb"), render(R"(module {{module}}
  def self.greeting
    "Hello from {{name}}!"
  end
end
)", {{"{{module}}", module}, {"{{name}}", name}})},
        {fs::path("test") / (lib + "_test.rb"), render(R"(require "minitest/autorun"
require "{{lib}}"

class {{module}}Test < Minitest::Test
  def test_greeting
    assert_equal "Hello from {{name}}!", {{module}}.greeting
  end
end
)", {{"{{lib}}", lib}, {"{{module}}", module}, {"{{name}}", name}})},
        {".gitignore", ".bundle/\nvendor/bundle/\ncoverage/\n"},
        {"README.md", "# " + name + "\n\nRuby starter project.\n\n```sh\nbundle install\nbundle exec rake\n```\n"},
    };
}

Files go_files(const std::string& name) {
    const std::string module = package_name(name);
    return {
        {"go.mod", "module example.com/" + module + "\n\ngo 1.22\n"},
        {"cmd/app/main.go", render(R"(package main

import "fmt"

func main() {
	fmt.Println("Hello from {{name}}!")
}
)", {{"{{name}}", name}})},
        {"internal/.gitkeep", ""},
        {".gitignore", "bin/\n*.test\ncoverage.out\n"},
        {"README.md", "# " + name + "\n\nGo starter project.\n\n```sh\ngo run ./cmd/app\ngo test ./...\n```\n"},
    };
}

Files rust_files(const std::string& name) {
    const std::string package = package_name(name);
    return {
        {"Cargo.toml", render(R"([package]
name = "{{package}}"
version = "0.1.0"
edition = "2021"

[dependencies]
)", {{"{{package}}", package}})},
        {"src/main.rs", "fn main() {\n    println!(\"Hello from " + name + "!\");\n}\n"},
        {".gitignore", "/target/\nCargo.lock\n"},
        {"README.md", "# " + name + "\n\nRust Cargo starter project.\n\n```sh\ncargo run\ncargo test\n```\n"},
    };
}

Files python_files(const std::string& name) {
    const std::string module = python_module(name);
    const std::string package = package_name(name);
    return {
        {"pyproject.toml", render(R"([project]
name = "{{package}}"
version = "0.1.0"
description = "Starter Python project"
readme = "README.md"
requires-python = ">=3.11"

[project.scripts]
{{package}} = "{{module}}.cli:main"

[build-system]
requires = ["setuptools>=68"]
build-backend = "setuptools.build_meta"
)", {{"{{package}}", package}, {"{{module}}", module}})},
        {fs::path("src") / module / "__init__.py", "__version__ = \"0.1.0\"\n"},
        {fs::path("src") / module / "cli.py", "def main() -> None:\n    print(\"Hello from " + name + "!\")\n\n\nif __name__ == \"__main__\":\n    main()\n"},
        {"tests/test_smoke.py", "from " + module + ".cli import main\n\n\ndef test_main(capsys):\n    main()\n    assert \"Hello from " + name + "!\" in capsys.readouterr().out\n"},
        {".gitignore", "__pycache__/\n*.py[cod]\n.venv/\nbuild/\ndist/\n*.egg-info/\n.pytest_cache/\n"},
        {"README.md", "# " + name + "\n\nPython starter project.\n\n```sh\npython -m pip install -e .\npython -m pytest\n```\n"},
    };
}

const std::unordered_map<std::string, TemplateFn> templates = {
    {"java", java_files}, {"maven", java_files},
    {"cpp", cpp_files}, {"c++", cpp_files},
    {"c", c_files},
    {"csharp", csharp_files}, {"c#", csharp_files}, {"cs", csharp_files},
    {"typescript", typescript_files}, {"ts", typescript_files},
    {"ruby", ruby_files}, {"rb", ruby_files},
    {"go", go_files}, {"golang", go_files},
    {"javascript", javascript_files}, {"js", javascript_files}, {"node", javascript_files},
    {"rust", rust_files}, {"rs", rust_files}, {"cargo", rust_files},
    {"python", python_files}, {"py", python_files},
};

void print_usage() {
    std::cout << "Usage:\n"
              << "  Lime init [language] [path/to/folder]\n\n"
              << "Languages:\n"
              << "  java, cpp, c, csharp, typescript, ruby, go, javascript, rust, python\n";
}

void write_files(const fs::path& root, const Files& files) {
    fs::create_directories(root);

    for (const FileSpec& file : files) {
        const fs::path target = root / file.path;
        if (fs::exists(target)) {
            throw std::runtime_error("refusing to overwrite existing file: " + target.string());
        }
    }

    for (const FileSpec& file : files) {
        const fs::path target = root / file.path;
        fs::create_directories(target.parent_path());
        std::ofstream stream(target, std::ios::binary);
        if (!stream) {
            throw std::runtime_error("failed to write file: " + target.string());
        }
        stream << file.content;
    }
}

int init_project(const std::string& language, const fs::path& root) {
    const auto found = templates.find(lower(language));
    if (found == templates.end()) {
        std::cerr << "Unsupported language: " << language << "\n\n";
        print_usage();
        return 1;
    }

    const std::string name = project_name_from_path(root);
    const Files files = found->second(name);
    write_files(root, files);

    std::cout << "Initialized " << language << " project in " << fs::absolute(root).string() << "\n";
    for (const FileSpec& file : files) {
        std::cout << "  " << (root / file.path).string() << "\n";
    }
    return 0;
}

int main(int argc, char** argv) {
    try {
        if (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h")) {
            print_usage();
            return 0;
        }

        if (argc != 4 || std::string_view(argv[1]) != "init") {
            print_usage();
            return argc == 1 ? 0 : 1;
        }

        return init_project(argv[2], argv[3]);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
}
