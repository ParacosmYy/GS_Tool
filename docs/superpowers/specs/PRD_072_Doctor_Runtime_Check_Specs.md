# PRD-072 - Doctor Runtime Check Specs

## 1. 目标

把 LOOP Protocol 中的 Doctor 系统体检变成可运行脚本，用于快速诊断本机开发环境和启动链路：

- 检查仓库根目录、`build/` 唯一性、`build/EmbedDebug.exe`。
- 检查 `local_env.bat` 中的 Qt、MinGW、CMake、Ninja 配置。
- 检查 PATH 中的 CMake、Ninja、Go 可用性。
- 可选执行 Qt 构建和 `EmbedDebug.bat` 启动验证。

## 2. 非目标

- 不自动写入 `local_env.bat`。
- 不自动安装工具链。
- 不自动停止正在运行的 `EmbedDebug.exe`。
- 不创建或兼容第二构建目录。
- 不修改 C++ 产品代码。

## 3. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/07-directory-structure.md`
- `docs/superpowers/LOOP_PROTOCOL.md`

## 4. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| Doctor 脚本 | `tools/doctor.ps1` | 新增 / 修改 |
| Specs | `docs/superpowers/specs/PRD_072_Doctor_Runtime_Check_Specs.md` | 新增 / 修改 |
| LOOP 文档 | `docs/superpowers/LOOP_PROTOCOL.md` | 修改 |
| 目录文档 | `docs/constraints/07-directory-structure.md` | 修改 |
| 禁止修改 | `src/`, `CMakeLists.txt`, `EmbedDebug.bat` | 禁止，除非进入新的构建线任务 |

## 5. 验收标准

- [ ] `tools/doctor.ps1` 默认只做只读诊断。
- [ ] `tools/doctor.ps1` 能输出 PASS/WARN/FAIL/SKIP 状态。
- [ ] 脚本能检查 `build/` 唯一性。
- [ ] 脚本能检查 `build/EmbedDebug.exe` 是否存在。
- [ ] 脚本能检查 Qt、MinGW、CMake、Ninja、Go。
- [ ] `-RunBuild` 可选执行 `cmake --build .\build --config Release --parallel 4`。
- [ ] `-RunLaunch` 可选执行 `.\EmbedDebug.bat`。
- [ ] Qt 构建和 `EmbedDebug.bat` 在收口时仍可验证通过。

## 6. 失败条件

- 脚本创建 `build2/`、`build-debug/`、`build-release/` 或 `cmake-build-*`。
- 脚本默认启动应用或修改环境文件。
- 脚本把缺少 Go 当作 Qt 主程序构建失败。
- 脚本隐藏失败命令或不输出修复建议。

## 7. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1"
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1 -RunBuild"
  ],
  "fix": []
}
```

## 8. BATCH 判定

- 是否需要 BATCH：否。
- 原因：本轮只新增一个脚本和两处文档登记，范围小且不适合并行写共享文档。
- 并行度上限：1。

## 9. LOOP 路由

- Doctor：本脚本自身。
- Debug：脚本检查失败后，按具体失败项进入最小复现。
- Simplify：如果 Doctor 输出开始膨胀，拆分为小 helper，但不引入复杂框架。
