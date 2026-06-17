# PRD-132 Python 启动面最小化

## 目标

将 Windows 用户入口与工程辅助入口收敛到 Python/PyQt 主线，避免保留多套脚本造成维护分叉。

## 范围

- `EmbedDebug.bat` 直接调用 `uv run start-embeddebug`。
- `tools/` 不再保留 PowerShell 启动或体检脚本。
- VSCode 项目配置只服务 Python/PyQt 编辑体验。
- README 与约束文档只描述 uv scripts、PyInstaller 和 Python/PyQt smoke。

## 非目标

- 不改变 Python/PyQt 应用运行时代码。
- 不改变 PyInstaller 打包实现。
- 不提升真实设备验证状态。

## 验收

- 根目录只保留 `EmbedDebug.bat` 作为用户启动入口。
- `tools/` 不包含 PowerShell 脚本。
- `EmbedDebug.bat --smoke` 能进入 Python/PyQt smoke。
- `uv run test-embeddebug-py` 与 `uv run test-embeddebug-tools` 通过。

## 三轴状态

- 工程状态：`E4`，脚本面有自动化门禁与 smoke 证据。
- 用户状态：`U3`，用户入口更短，仍保持双击和命令行可启动。
- 设备状态：`D1`，本轮只涉及启动面和工程治理。
