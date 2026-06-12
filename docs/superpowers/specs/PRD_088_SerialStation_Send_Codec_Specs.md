# PRD-088 - Serial Station 发送编码器

## 1. 标题

`PRD-088 - Serial Station 发送编码器`

## 2. 目标

- 新增 `SerialCodec`，承接 ASCII、HEX、协议模式的发送帧构建。
- Controller 从“模式分支 + 字节构建”收敛为“调用 codec + 发送结果处理”。
- UI 下拉补齐 HEX 模式，和输入提示一致。

## 3. 非目标

- 不新增真实串口硬件依赖。
- 不新增 Modbus/custom 协议。
- 不修改旧 `src/serial/`。
- 不调整 QSS 视觉。
- 不修改 `utils/crypto/HexConverter.h`。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/05-ui-standard.md`
- `docs/constraints/06-git-commit.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要文件 | `src/apps/serial_station/core/SerialCodec.h/.cpp` | 新增 |
| 主要文件 | `src/apps/serial_station/SerialStationController.h/.cpp` | 修改 |
| UI 文件 | `src/apps/serial_station/ui/SerialCommandPanel.cpp` | 修改 |
| 测试 | `tests/serial_station/test_serial_codec.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_station_controller.cpp` | 修改 |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 必要注册 |
| 禁止修改 | `src/serial/` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 6. 验收标准

- [ ] `SerialCodec` 覆盖 ASCII、HEX、协议、未知模式。
- [ ] HEX 解析覆盖空格、逗号、换行、`0x` 前缀、奇数字符、非法字符。
- [ ] Controller `hex` 模式不再被拒绝，能发出 `serialCommandPrepared`。
- [ ] `SerialCommandPanel` 包含 HEX 模式选项，用户可选择。
- [ ] 新增 `.h/.cpp` 已注册主 CMake 和测试 CMake。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "SerialCodec|SerialStationController|SerialCommandPanel|SerialStationWorkbench" --output-on-failure` 通过。
- [ ] `tools/doctor.ps1` 通过。
- [ ] `EmbedDebug.bat` 可启动。

## 7. 失败条件

- Controller 继续扩张具体编码细节。
- UI 直接构造发送 bytes。
- `core/` include 具体协议目录。
- 验证命令不能覆盖 HEX 模式。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build build --target EmbedDebug --parallel 4",
    "ctest --test-dir build -R \"SerialCodec|SerialStationController|SerialCommandPanel|SerialStationWorkbench\" --output-on-failure"
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "git diff --cached --numstat -- '*.cpp' '*.h' '*.cc' '*.cxx' '*.c' '*.hpp'"
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：否。
- 子任务数量：不适用。
- 并行度上限：1。
- 人工审查状态：不适用，本轮触碰共享发送路径，串行更合适。

## 10. LOOP 路由

- Doctor：构建、测试、bat 启动失败时先跑 `tools/doctor.ps1`。
- Debug：HEX 或协议断言失败时缩小到 `test_serial_codec`。
- Simplify：若 Controller 模式分支继续膨胀，回收进 `SerialCodec`。
