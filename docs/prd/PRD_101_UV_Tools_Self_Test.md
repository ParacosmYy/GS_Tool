# PRD_101 - UV Tools Self Test

## 1. 背景

仓库已经有 `uv run package-embeddebug` 和 `uv run start-embeddebug` 两个快捷工具。它们属于启动/打包链路，一旦入口坏掉，用户会直接无法启动或发布。因此需要一个轻量自测命令，用于验证 Python 工具的关键纯逻辑和入口可发现性。

## 2. 目标

1. 新增 `uv run test-embeddebug-tools`。
2. 使用 Python 标准库 `unittest`，不引入额外依赖。
3. 覆盖 `start_embeddebug` 的 repo root、bat 路径、dry-run 命令。
4. 覆盖 `package_embeddebug` 的包名生成、local_env 解析、zip 创建纯逻辑。
5. README 增加工具自测命令。
6. 不运行真实构建、不启动 GUI、不调用 `windeployqt`。

## 3. 非目标

- 不替代 CTest/QTest。
- 不测试真实 Qt 部署。
- 不启动 `EmbedDebug.exe`。
- 不新增第三方 Python 依赖。
- 不创建第二构建目录。

## 4. 验收标准

- `uv run test-embeddebug-tools` 通过。
- 测试不依赖真实 COM 口、Qt GUI 或网络。
- 测试不会写入 build 目录。
- README 同步自测命令。
- `EmbedDebug.bat` 探针仍通过。

## 5. 失败条件

- 自测命令需要 pytest 等未声明依赖。
- 测试会真实启动 GUI。
- 测试写入构建产物。
- 自测无法在 uv 虚拟环境内发现 tools 包。
