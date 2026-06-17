# LOOP Protocol

## 1. 目的

LOOP 用于持续迭代时的问题分层，避免所有失败都被当作“继续写代码”处理。

三层入口：

- Doctor：系统体检。
- Debug：追踪具体 Bug。
- Simplify：清理和降复杂度。

LOOP 不直接扩大需求范围。它只回答三个问题：

1. 当前失败属于环境/系统问题、具体缺陷，还是复杂度问题。
2. 需要哪条最小命令或报告证明问题。
3. 修复后用哪条回归命令收口。

GO 循环触发 20 轮或 30 分钟安全刹车后，必须进入 LOOP，不允许继续盲目增加 fix 命令。

## 2. Doctor

适用：

- 构建变慢或构建失败。
- `EmbedDebug.bat` 启动失败。
- 工具链、PATH、PyQt/PyInstaller 环境异常。

默认动作：

1. 检查 `git status --short`。
2. 检查 `uv` 与 Python/PyQt 依赖是否可用。
3. 检查 PyInstaller 打包入口和 package verify 入口。
4. 只在显式传入 `-RunTests` 时运行 `uv run test-embeddebug-py`。
5. 只在显式传入 `-RunLaunch` 时运行 `EmbedDebug.bat --smoke`。
6. 输出 PASS/WARN/FAIL 摘要。

本地入口：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 -RunTests
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 -RunTests -RunLaunch
```

默认 Doctor 只做诊断，不写入本地环境文件，不创建第二构建目录，不启动应用。

失败后进入 Debug，不直接扩大代码改动。

Doctor 收口要求：

- 如果失败是环境缺失，输出缺失组件和修复命令。
- 如果失败是测试错误，复制最后一段 pytest 错误进入 Debug。
- 如果失败是启动错误，记录启动命令、退出码、窗口/进程观察结果。

## 3. Debug

适用：

- 可复现崩溃。
- 编译错误。
- 逻辑错误。
- UI 行为明显错误。

默认动作：

1. 记录复现命令和错误输出。
2. 定位最小文件范围。
3. 做最小修复。
4. 重跑触发失败的命令。
5. 若修复引入复杂度，转入 Simplify。
6. 若修复涉及新行为或 bugfix，必须有回归测试或明确不能自动化的人工回归步骤。

本地入口：

```powershell
uv run test-embeddebug-py
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

如需归档报告，手写 Markdown 只能写入 `docs/reviews/debug/`。

Debug 收口要求：

- 复现命令必须可复制执行。
- 候选文件必须尽量收敛，不能用“全项目”代替定位。
- 回归命令必须覆盖原始失败。
- 修复不得越过 PRD/Specs 禁止文件范围。

## 4. Simplify

适用：

- 重复实现。
- 文件过大。
- 多层职责混在一起。
- 历史分叉继续扩张。

默认动作：

1. 写清行为不变范围。
2. 找到 canonical 落点。
3. 小步拆分或删除重复。
4. 保持外部接口稳定。
5. 运行必要测试和启动 smoke。
6. 如果只做结构清理，不得提升用户状态或设备状态。

本地入口：

```powershell
uv run test-embeddebug-py
uv run test-embeddebug-tools
```

Simplify 证据必须来自可复现测试、只读审计或人工写明的 Markdown 记录。归档报告只能写入 `docs/reviews/simplify/`。扫描结果只是选择下一步清理目标的证据，不代表可以跳过 PRD、技术债说明或验证。

Simplify 收口要求：

- 说明行为不变范围。
- 删除或迁移重复实现时说明 canonical 路径。
- 文件拆分后保持外部接口稳定。
- 不把清理任务扩展成新功能。

## 5. 安全刹车

每个 GO 循环最多：

- 20 轮。
- 30 分钟。

触发任意刹车后必须停止当前循环，输出最后失败命令、最后错误摘要和下一步建议。

刹车后路由：

| 最后失败位置 | 路由 |
|--------------|------|
| `execute` 失败 | Doctor；如果是测试或运行时错误再进入 Debug |
| `check` 失败且无 fix | Debug；先补复现和回归命令 |
| `fix` 失败 | Debug；定位 fix 命令本身或候选文件 |
| 多轮 fix 后仍失败 | Simplify；检查是否任务过大、边界错误或重复实现 |
| 超过 30 分钟 | Doctor；先排查环境和进程卡死 |

## 6. 输出证据

每次 LOOP 结束必须至少留下一个证据：

- 控制台输出摘要。
- `docs/reviews/debug/*.md` 调试报告。
- `docs/reviews/simplify/*.md` 简化扫描报告。
- PRD/Specs 中的收口记录。

没有证据的 LOOP 不能作为完成依据。
