# PRD-090 - Serial Station Custom MD 协议

## 1. 标题

`PRD-090 - Serial Station Custom MD 协议`

## 2. 目标

- 在 Serial Station 协议层新增 `custom_md`，补齐第一批协议集合。
- 支持默认 MCU 自定义帧构建和接收流式解析。
- 用 QTest 锁住参数校验、checksum、半包、粘包、噪声重同步和 registry 创建行为。

## 3. 非目标

- 不新增 UI 参数配置面板。
- 不实现通用协议 DSL。
- 不接入真实串口硬件。
- 不修改旧 `src/serial/` 或旧 `src/protocol/`。
- 不调整 QSS 或窗口布局。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/06-git-commit.md`
- `docs/constraints/07-directory-structure.md`
- `docs/serial_station_architecture.md`
- `C:/Users/LWH/.agents/skills/mcu/references/docs/part1_code_style.md`
- `C:/Users/LWH/.agents/skills/mcu/references/docs/checklist.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要文件 | `src/apps/serial_station/protocols/custom_md/CustomMdProtocol.h/.cpp` | 新增 |
| 主要文件 | `src/apps/serial_station/protocols/SerialProtocolRegistry.cpp` | 修改 |
| 主要文件 | `src/apps/serial_station/SerialStationConstants.h` | 修改 |
| 测试 | `tests/serial_station/test_custom_md_protocol.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_protocol_registry.cpp` | 修改 |
| 构建 | `CMakeLists.txt`, `tests/CMakeLists.txt` | 必要注册 |
| 禁止修改 | `src/serial/` | 禁止 |
| 禁止修改 | `src/apps/serial_station/core/` | 禁止 |
| 禁止修改 | `src/apps/serial_station/ui/` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 6. 默认帧格式

```text
A5 5A | len | cmd | payload... | checksum | 0D 0A
```

| 字段 | 长度 | 说明 |
|------|------|------|
| `header` | 2 | 默认 `A5 5A` |
| `len` | 1 | `cmd + payload` 字节数 |
| `cmd` | 1 | 命令码，范围 `0x00..0xFF` |
| `payload` | 0..240 | 二进制负载 |
| `checksum` | 1 | `len/cmd/payload` 低 8 位加和 |
| `footer` | 2 | 默认 `0D 0A` |

## 7. 参数规则

- `command` 可以是十进制、`0x` 十六进制或两位 HEX 字符串。
- `params["payload"]` 可以是 `QByteArray` 或 HEX 字符串。
- `params["header"]`、`params["footer"]` 可选，支持 `QByteArray` 或 HEX 字符串。
- `params["checksum"]` 可选，默认 `sum8`；允许 `none`。
- payload 超过 240 字节时返回失败和明确错误。

## 8. 接收解析

- 缓存小于最短帧长度时继续等待。
- 找不到 header 时丢弃 header 前噪声。
- 长度不足时继续缓存。
- footer 不匹配或 checksum 错误时输出 `custom_md_error`，并向后移动一字节重同步。
- 成功解析时输出 `custom_md_frame`，payload 包含 `commandCode`、`payload`、`length`、`checksum`。

## 9. 验收标准

- [ ] `SerialProtocolRegistry` 可创建 `custom_md`。
- [ ] 默认构帧结果匹配 `A5 5A 03 10 01 02 16 0D 0A`。
- [ ] `payload` 的 `QByteArray` 和 HEX 字符串路径均被测试覆盖。
- [ ] 非法命令、非法 HEX、payload 超长返回失败和明确错误。
- [ ] 半包不产出事件，补齐后产出一条业务事件。
- [ ] 粘包能产出两条业务事件。
- [ ] 噪声前缀后能重同步到合法帧。
- [ ] checksum 错误产出 `custom_md_error`。
- [ ] 新增 `.h/.cpp` 已注册主 CMake 和测试 CMake。
- [ ] `cmake --build build --target test_custom_md_protocol test_serial_protocol_registry EmbedDebug --parallel 4` 通过。
- [ ] `ctest --test-dir build -R "CustomMdProtocol|SerialProtocolRegistry" --output-on-failure` 通过。
- [ ] `tools/doctor.ps1` 通过。
- [ ] `EmbedDebug.bat` 可启动。

## 10. 失败条件

- 协议层 include UI/Controller/Window。
- `core/` 反向 include `protocols/custom_md`。
- checksum 错误帧被当成正常帧。
- 只改 CMake 不落协议代码。

## 11. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 35,
  "execute": [
    "cmake --build build --target test_custom_md_protocol test_serial_protocol_registry EmbedDebug --parallel 4",
    "ctest --test-dir build -R \"CustomMdProtocol|SerialProtocolRegistry\" --output-on-failure"
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1",
    "git diff --cached --numstat -- '*.cpp' '*.h' '*.cc' '*.cxx' '*.c' '*.hpp'"
  ],
  "fix": []
}
```

## 12. BATCH 判定

- 是否需要 BATCH：否。
- 子任务数量：不适用。
- 并行度上限：1。
- 人工审查状态：不适用，本轮新增单个协议并触碰 registry，串行实现更稳。

## 13. LOOP 路由

- Doctor：构建、测试、bat 启动失败时先跑 `tools/doctor.ps1`。
- Debug：构帧或解析失败时缩小到 `test_custom_md_protocol`。
- Simplify：如果协议 `.cpp` 接近 500 行，拆分 helper 或收缩可配置范围。
