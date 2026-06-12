# PRD_099 - UV Package Tool

## 1. 背景

用户希望像 Python 上位机项目一样，通过 `uv run ...` 一条命令完成启动或打包，并提到 `uv run pyinstaller` 这类工作流。EmbedDebug 本体是 C++/Qt 程序，不能用 PyInstaller 直接把 C++ 源码打包成 exe；正确做法是提供一个 Python/uv 快捷工具，统一编排 CMake 构建、Qt runtime 部署和分发目录/zip 生成。

## 2. 目标

1. 新增 `pyproject.toml`，提供 `uv run package-embeddebug` 命令入口。
2. 新增 `tools/package_embeddebug.py`，执行可重复的 Windows 打包流程。
3. 打包流程只使用唯一构建目录 `build/`。
4. 自动读取 `local_env.bat` 中的 `QT_PREFIX`、`MINGW_BIN`、`CMAKE_BIN`、`NINJA_BIN`。
5. 支持构建 `EmbedDebug.exe`。
6. 使用 Qt `windeployqt.exe` 部署运行时。
7. 输出到 `dist/EmbedDebug-<version>-windows-x64/`。
8. 支持 `--zip` 生成 zip 包。
9. 支持 `--skip-build` 复用已有 `build/EmbedDebug.exe`。
10. README 补充 `uv run` 打包入口。

## 3. 非目标

- 不使用 PyInstaller 打包 C++/Qt 主程序。
- 不引入第二构建目录。
- 不提交 dist 产物。
- 不制作安装器 UI。
- 不修改 CMake 架构。
- 不改变 `EmbedDebug.bat` 启动链路。

## 4. 验收标准

- `uv run package-embeddebug --help` 可运行。
- `uv run package-embeddebug --skip-build` 能从已有 build 产物生成 dist 目录。
- dist 目录包含 `EmbedDebug.exe`、Qt runtime、README 和配置样例。
- `--zip` 能生成 zip 文件。
- `.gitignore` 排除 `dist/`。
- 打包失败时给出明确错误信息。
- `EmbedDebug.bat` 验证仍通过。

## 5. 失败条件

- 创建 `build2/`、`build-release/` 等平行构建目录。
- 把 dist 产物或 exe/dll 提交进 Git。
- 脚本要求用户手动复制 Qt DLL。
- 脚本把 PyInstaller 当作 C++ 主程序打包器。
