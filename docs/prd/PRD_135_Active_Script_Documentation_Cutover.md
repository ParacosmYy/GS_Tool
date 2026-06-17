# PRD-135 Active Script Documentation Cutover

## 目标

把活跃治理文档中的脚本入口口径收敛到当前 Python/PyQt 主线，避免继续指导使用已删除的本地 shell 脚本。

## 范围

- 审计活跃脚本面，确认源码树只保留根目录 `EmbedDebug.bat`。
- 更新 LOOP 文档中的 Doctor 入口，改为 `uv` 与 `EmbedDebug.bat` smoke 命令。
- 增加测试门禁，禁止活跃治理文档重新引用已删除的脚本入口。

## 非目标

- 不改变 PyQt 应用运行时代码。
- 不新增 shell 脚本。
- 不改变 PyInstaller 打包实现。

## 验收

- 活跃治理文档不再引用已删除的 shell 脚本。
- 根目录仍只保留 `EmbedDebug.bat` 作为用户启动入口。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，脚本口径由测试门禁锁定。
- 用户状态：`U3`，用户入口保持单一且文档不再误导。
- 设备状态：`D1`，本轮仅涉及工程治理与文档入口。
