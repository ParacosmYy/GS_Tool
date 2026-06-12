# PRD_100 - UV Start Tool

## 1. 背景

仓库已经提供 `EmbedDebug.bat` 作为最低启动入口，也新增了 `uv run package-embeddebug` 作为打包快捷入口。用户希望后续保持“像上位机工程一样”的快捷工具体验，因此启动和打包都应能通过 uv 命令进入。

本阶段新增 `uv run start-embeddebug`，作为 `EmbedDebug.bat` 的 Python/uv 包装入口。它不替代 bat，也不创建第二套启动逻辑，只负责从 uv 工作流调用既有启动链路。

## 2. 目标

1. 在 `pyproject.toml` 注册 `start-embeddebug`。
2. 新增 `tools/start_embeddebug.py`。
3. 默认调用仓库根目录 `EmbedDebug.bat`。
4. 支持 `--wait` 等待进程结束，便于自动化脚本获取退出码。
5. 支持 `--dry-run` 打印将执行的命令。
6. README 补充 uv 启动命令。
7. 不改变 `EmbedDebug.bat` 自身职责。

## 3. 非目标

- 不新增第二构建目录。
- 不绕过 `EmbedDebug.bat`。
- 不重写 `tools/launch_embeddebug.ps1`。
- 不修改 C++ 生产代码。
- 不提交运行产物。

## 4. 验收标准

- `uv run start-embeddebug --help` 可运行。
- `uv run start-embeddebug --dry-run` 输出 `EmbedDebug.bat` 路径。
- 默认启动路径仍依赖 `EmbedDebug.bat`。
- `EmbedDebug.bat` 启动探针仍通过。
- README 同时展示 `uv run start-embeddebug` 和 `uv run package-embeddebug`。

## 5. 失败条件

- uv 启动工具绕开 bat 直接拼接 build exe。
- 工具创建或引用 `build2/` 等平行目录。
- 启动工具依赖 dist 产物。
- 新增命令无法被 `uv run` 发现。
