# PRD_102 - UV Package Verify Tool

## 1. 背景

`uv run package-embeddebug` 已经可以把 `build/EmbedDebug.exe` 结合 Qt `windeployqt` 输出到 `dist/`。但当前缺少一个离线校验入口，用于确认打包目录是否具备最基础的可分发结构。若只依赖人工查看目录，容易漏掉 exe、Qt platform 插件、README 或随包文档。

## 2. 目标

1. 新增 `uv run verify-package-embeddebug`。
2. 默认校验 `dist/` 下最新的 `EmbedDebug-*-windows-x64` 目录。
3. 支持 `--package-dir` 指定待校验目录。
4. 校验 `EmbedDebug.exe`、Qt 核心运行库、`platforms/qwindows.dll`、README 和随包约束文档。
5. 输出清晰的 `ok:` / `missing:` 检查结果，失败时返回非 0 退出码。
6. 接入 `uv run test-embeddebug-tools` 的纯逻辑测试。
7. README 同步新增发布校验命令。

## 3. 非目标

- 不启动 GUI。
- 不运行 CMake 构建。
- 不调用 `windeployqt`。
- 不创建或兼容第二构建目录。
- 不替代真实用户机器上的冒烟运行。

## 4. 验收标准

- `uv run verify-package-embeddebug --help` 可用。
- 对缺失文件的临时目录返回失败并列出缺失项。
- 对包含最小必需文件的临时目录返回成功。
- `uv run test-embeddebug-tools` 覆盖校验工具纯逻辑并通过。
- 如果本机已有打包目录，能用 `--package-dir` 对其完成离线校验。
- `EmbedDebug.bat` 探针仍通过。

## 5. 失败条件

- 校验工具需要真实启动 `EmbedDebug.exe`。
- 校验工具写入 `build/` 或重新生成 Qt 部署文件。
- 默认选择目录时引用 `build2/`、`build-debug/`、`build-release/` 等平行构建目录。
- README 写入无法执行或与当前 `uv` 入口不一致的命令。
