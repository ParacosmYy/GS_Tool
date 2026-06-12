# PRD-089 - Serial Station Modbus RTU 协议

## 1. 标题

`PRD-089 - Serial Station Modbus RTU 协议`

## 2. 目标

- 在 Serial Station 协议层新增 `modbus_rtu`，补齐当前协议目录缺口。
- 支持基础 Modbus RTU 主站请求构建和接收帧解析。
- 用 QTest 锁住 CRC、长度推断、半包、粘包、异常响应和 registry 创建行为。

## 3. 非目标

- 不新增 Modbus UI 参数表单。
- 不接入真实串口硬件。
- 不实现从站寄存器映射。
- 不调整 QSS 或窗口布局。
- 不修改旧 `src/serial/`。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/06-git-commit.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`
- `C:/Users/LWH/.agents/skills/mcu/references/methodology/65_can_modbus.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要文件 | `src/apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.h/.cpp` | 新增 |
| 主要文件 | `src/apps/serial_station/protocols/SerialProtocolRegistry.cpp` | 修改 |
| 主要文件 | `src/apps/serial_station/SerialStationConstants.h` | 修改 |
| 测试 | `tests/serial_station/test_modbus_rtu_protocol.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_protocol_registry.cpp` | 修改 |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 必要注册 |
| 禁止修改 | `src/serial/` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 6. 协议命令

| 命令 | 功能码 | 必需参数 | 范围 |
|------|--------|----------|------|
| `read_coils` | `0x01` | `slaveId`, `startAddress`, `quantity` | `quantity` 1..2000 |
| `read_discrete_inputs` | `0x02` | `slaveId`, `startAddress`, `quantity` | `quantity` 1..2000 |
| `read_holding_registers` | `0x03` | `slaveId`, `startAddress`, `quantity` | `quantity` 1..125 |
| `read_input_registers` | `0x04` | `slaveId`, `startAddress`, `quantity` | `quantity` 1..125 |
| `write_single_coil` | `0x05` | `slaveId`, `address`, `value` | `value` bool 或 `0x0000/0xFF00` |
| `write_single_register` | `0x06` | `slaveId`, `address`, `value` | `value` 0..65535 |

## 7. 接收解析

- 最小帧长小于 5 字节时继续缓存。
- `functionCode | 0x80` 的异常响应长度固定为 5 字节。
- FC01/FC02/FC03/FC04 响应按 byte count 推断总长度：`3 + byteCount + 2`。
- FC05/FC06 请求或响应固定 8 字节。
- CRC 正确时输出 `modbus_frame` 事件，payload 包含从站地址、功能码、数据、异常码等字段。
- CRC 错误时输出 `modbus_error` 事件，并丢弃当前坏帧，避免无限卡住缓存。

## 8. 验收标准

- [ ] `SerialProtocolRegistry` 可创建 `modbus_rtu`。
- [ ] FC03 请求构建结果匹配已知 CRC。
- [ ] FC06 请求构建结果匹配已知 CRC。
- [ ] 参数越界返回失败和明确错误。
- [ ] 半包不产出事件，补齐后产出一条事件。
- [ ] 粘包能产出两条业务事件。
- [ ] 异常响应产出业务事件并带 exception code。
- [ ] CRC 错误产出 `modbus_error`。
- [ ] 新增 `.h/.cpp` 已注册主 CMake 和测试 CMake。
- [ ] `cmake --build build --target test_modbus_rtu_protocol test_serial_protocol_registry EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "ModbusRtuProtocol|SerialProtocolRegistry" --output-on-failure` 通过。
- [ ] `tools/doctor.ps1` 通过。
- [ ] `EmbedDebug.bat` 可启动。

## 9. 失败条件

- 协议层 include UI/Controller/Window。
- `core/` 反向 include `protocols/modbus_rtu`。
- CRC 错误帧被当成正常帧。
- 只改 CMake 不落协议代码。

## 10. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 35,
  "execute": [
    "cmake --build build --target test_modbus_rtu_protocol test_serial_protocol_registry EmbedDebug --parallel 4",
    "ctest --test-dir build -R \"ModbusRtuProtocol|SerialProtocolRegistry\" --output-on-failure"
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "git diff --cached --numstat -- '*.cpp' '*.h' '*.cc' '*.cxx' '*.c' '*.hpp'"
  ],
  "fix": []
}
```

## 11. BATCH 判定

- 是否需要 BATCH：否。
- 子任务数量：不适用。
- 并行度上限：1。
- 人工审查状态：不适用，本轮新增一个协议并触碰 registry，串行实现更容易保证边界。

## 12. LOOP 路由

- Doctor：构建、测试、bat 启动失败时先跑 `tools/doctor.ps1`。
- Debug：CRC 或解析失败时缩小到 `test_modbus_rtu_protocol`。
- Simplify：如果协议 `.cpp` 接近 500 行，拆出纯内部 helper 或降低命令范围。
