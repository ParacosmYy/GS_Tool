# Specs - PRD-078 CMake Generated Utils Slimming

## 1. 标题

`PRD-078 - CMake Generated Utils Slimming`

## 2. 目标

- 扩展 CMake 过滤规则，减少主 GUI 目标参与编译的批量算法草稿。
- 不删除源码，不移动目录。
- 让后续全量构建更接近可完成状态。

## 3. 非目标

- 不改生产 C++ 业务逻辑。
- 不接入新 UI。
- 不删除 `src/utils`。
- 不创建第二构建目录。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/07-directory-structure.md`
- `docs/prd/PRD_076_Source_Tree_Slimming_And_SerialStation_Minimal_Uart.md`
- `docs/prd/PRD_078_CMake_Generated_Utils_Slimming.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 构建 | `CMakeLists.txt` | 修改现有 `list(FILTER ...)` 规则 |
| 文档 | `docs/prd/`, `docs/superpowers/specs/` | 新增 PRD/Specs |
| 审计 | `docs/reviews/simplify/source-tree-latest.md` | 刷新报告 |
| 禁止修改 | `src/` 生产源码 | 禁止 |
| 禁止修改 | `EmbedDebug.bat`、`tools/launch_embeddebug.ps1` | 禁止 |

## 6. 验收标准

- [ ] `cmake -S . -B build -DBUILD_TESTS=ON -DCMAKE_PREFIX_PATH=C:/msys64/mingw64` 通过。
- [ ] `ctest --test-dir build -R "AsciiTextProtocol|SerialProtocolRegistry|SerialSession"` 通过。
- [ ] `ninja -C build -t targets all` 不再包含 `src/utils/aes2/`、`src/utils/btree5/`、`src/utils/cache2/`、`src/utils/linalg6/` 等数字目录对象。
- [ ] `powershell -File tools/doctor.ps1` 通过。
- [ ] `EmbedDebug.bat` 可启动已有 exe。

## 7. 失败条件

- CMake 配置失败。
- 新增测试失败。
- 过滤规则误伤 canonical 工具目录。
- 需要修改生产源码来适配过滤。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake -S . -B build -DBUILD_TESTS=ON -DCMAKE_PREFIX_PATH=C:/msys64/mingw64",
    "ctest --test-dir build -R \"AsciiTextProtocol|SerialProtocolRegistry|SerialSession\" --output-on-failure"
  ],
  "check": [
    "ninja -C build -t targets all",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\source-tree-audit.ps1 -OutFile .\\docs\\reviews\\simplify\\source-tree-latest.md"
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 原因：本轮只改共享 CMake 合流点，必须串行。

## 10. LOOP 路由

- Doctor：环境、CMake、bat 启动问题。
- Debug：CMake 过滤误伤导致配置/链接失败。
- Simplify：继续拆分 utils 或 feature widget。
