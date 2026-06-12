# LOOP Protocol

## 1. 目的

LOOP 用于持续迭代时的问题分层，避免所有失败都被当作“继续写代码”处理。

三层入口：

- Doctor：系统体检。
- Debug：追踪具体 Bug。
- Simplify：清理和降复杂度。

## 2. Doctor

适用：

- 构建变慢或构建失败。
- `EmbedDebug.bat` 启动失败。
- 工具链、PATH、Qt 部署、CMake/Ninja 环境异常。

默认动作：

1. 检查 `git status --short`。
2. 检查 `build/EmbedDebug.exe` 是否存在。
3. 检查 Qt、CMake、Ninja、MinGW 路径。
4. 运行 `cmake --build .\build --config Release --parallel 4`。
5. 受影响时运行 `.\EmbedDebug.bat`。

本地入口：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 -RunBuild
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 -RunBuild -RunLaunch
```

默认 Doctor 只做诊断，不写入 `local_env.bat`，不创建第二构建目录，不启动应用。

失败后进入 Debug，不直接扩大代码改动。

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

本地入口：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\debug-trace.ps1 `
  -Title "问题标题" `
  -Severity P2 `
  -ReproCommand "复现命令" `
  -ErrorSummary "错误摘要" `
  -ImpactScope "影响范围" `
  -CandidateFiles "src/example.cpp" `
  -RegressionCommand "回归命令"
```

如需归档报告，`-OutFile` 只能写入 `docs/reviews/debug/`。

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
5. 运行构建和必要测试。

本地入口：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\simplify-scan.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\simplify-scan.ps1 -OutFile docs\reviews\simplify\latest.md
```

默认 Simplify 扫描只读输出 Markdown。归档报告只能写入 `docs/reviews/simplify/`。扫描结果只是选择下一步清理目标的证据，不代表可以跳过 PRD、技术债说明或构建验证。

## 5. 安全刹车

每个 GO 循环最多：

- 20 轮。
- 30 分钟。

触发任意刹车后必须停止当前循环，输出最后失败命令、最后错误摘要和下一步建议。
