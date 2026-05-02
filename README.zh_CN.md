# Slnx2CompileCommands(s2cc)

![Version](https://img.shields.io/badge/version-1.0.0-green)
![GitHub Repo stars](https://img.shields.io/github/stars/LaughingNailoong/slnx2compilecmd?style=flat)
![GitHub last commit (branch)](https://img.shields.io/github/last-commit/LaughingNailoong/slnx2compilecmd/main)



一个转换Visual Studio 2026解决方案到用于clangd的`compile_commands.json`编译数据库的工具。

## 使用方法
#### 示例

```Shell
s2cc ./app.slnx --outdir . -D Configuration=Debug -D Platform=x64
```

#### 参数

**基础语法:**
`s2cc [path] ...`
path: 你的解决方案文件路径

`--outdir [path]` 设置输出`compile_commands.json`文件的路径

`-D [variable]=[value]` 设置一个变量(e.g.,`-D Configuration=Debug`)

> [!IMPORTANT]
> 通常，我们需要设置Configuration和Platform变量来让s2cc选择正确的配置，否则它不会输出任何东西。


`--version` 输出版本信息
`--help` 显示帮助信息

## 安装

从Releases下载可执行文件并添加到`PATH`环境变量。