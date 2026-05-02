# Slnx2CompileCommands(s2cc)

![Version](https://img.shields.io/badge/version-1.0.0-green)
![GitHub Repo stars](https://img.shields.io/github/stars/LaughingNailoong/slnx2compilecmd?style=flat)
![GitHub last commit (branch)](https://img.shields.io/github/last-commit/LaughingNailoong/slnx2compilecmd/main)
[简体中文](./README.zh_CN.md)

A tool that converts Visual Studio 2026 solution files(.slnx) into `compile_commands.json` compilation database.

## Usage

#### Arguments

**Basic usage:**
`s2cc [path] ...`

path: Path to your solution file.

`--outdir [path]` sets the directory to save the `compile_commands.json` file.

`-D [variable]=[value]` sets the value of a variable(e.g.,`-D Configuration=Debug`).

> [!IMPORTANT]
> Usually, we have to specify the values of variables Configuration(e.g,Debug, Release) and Platform(e.g,x64, Win32) to make s2cc select correct configuration; otherwise, it won't output anything.

`--version` prints version information.

`--help` shows help message

#### Example

```Shell
s2cc ./app.slnx --outdir . -D Configuration=Debug -D Platform=x64
```

## Installation

Download the executable file from releases and add it to `PATH`.
