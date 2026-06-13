# PRD-118 - Serial Station Profile Service Specs

## 1. 标题

`PRD-118 - Serial Station Profile Service`

## 2. 目标

- 为 Serial Station 增加服务层配置档案能力。
- 档案覆盖 UART 配置、协议、发送模式、常用命令、标签和描述。
- 支持 JSON 文本和文件导入导出。
- 本轮三轴目标：工程 `E3 -> E4`，用户不提升，设备不提升。

## 3. 非目标

- 不新增 UI。
- 不修改 controller/window/core/protocols/workers。
- 不触发真实串口连接。
- 不把档案能力宣传为真实硬件闭环。

## 4. 约束

- 必读并遵守 `CLAUDE.md`、`01-project-overview.md`、`02-workflow.md`、`03-architecture.md`、`04-coding-standard.md`、`07-directory-structure.md`、`serial_station_architecture.md`。
- 单次 Serial Station 改动限定在 `services/` 层和对应测试/CMake/文档。
- 新增源码必须注册到 CMake。
- 构建目录只能是 `build/`。

## 5. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| PRD | `docs/prd/PRD_118_SerialStation_Profile_Service.md` | 新增 |
| Specs | `docs/superpowers/specs/PRD_118_SerialStation_Profile_Service_Specs.md` | 新增 |
| service | `src/apps/serial_station/services/SerialProfileService.h/.cpp` | 新增 |
| 测试 | `tests/serial_station/test_serial_profile_service.cpp` | 新增 |
| CMake | `cmake/EmbedDebugSources.cmake`, `tests/CMakeLists.txt` | 注册 |
| README | `README.md` | 检查是否需要同步 |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 收口更新 |

## 6. 设计

### 数据对象

`SerialStationProfile`:

- `schemaVersion`
- `name`
- `description`
- `tags`
- `SerialPortConfig port`
- `protocolName`
- `sendMode`
- `commands`

`SerialProfileCommand`:

- `name`
- `payload`
- `mode`

`SerialProfileResult`:

- `ok`
- `profile`
- `errorMessage`

`SerialProfileWriteResult`:

- `ok`
- `filePath`
- `bytesWritten`
- `errorMessage`

### JSON 格式

顶层字段：

```json
{
  "schemaVersion": 1,
  "name": "STM32 Bootloader",
  "description": "UART 115200 8N1",
  "tags": ["stm32", "bootloader"],
  "port": {
    "portName": "COM7",
    "baudRate": 115200,
    "dataBits": 8,
    "parity": "none",
    "stopBits": "1",
    "flowControl": "none",
    "dtrEnabled": true,
    "rtsEnabled": false
  },
  "protocolName": "modbus_rtu",
  "sendMode": "protocol",
  "commands": [
    {"name": "Read Version", "payload": "read_version", "mode": "ascii"}
  ]
}
```

## 7. 验收标准

- [x] 红灯测试先失败，失败原因是 `SerialProfileService` 尚不存在。
- [x] `test_serial_profile_service` 构建通过。
- [x] `test_serial_profile_service` QTest 通过。
- [x] `EmbedDebug` 主目标构建通过。
- [x] service include 边界检查无越界命中。
- [x] `EmbedDebug.bat` 启动探针通过。
- [x] 根目录只存在 `build/`。
- [ ] 本轮 commit。

## 8. 失败条件

- service include UI、controller、core/SerialManager 或具体协议目录。
- 新增源码未进 CMake。
- 文件保存错误被静默吞掉。
- 坏 JSON 或非法配置被当作成功。
- README 宣传超过当前三轴状态。

## 9. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E4`，service QTest 12/12 passed，主目标构建 passed |
| 用户状态 | 不提升 |
| 设备状态 | 不提升 |
| 验证命令 | `test_serial_profile_service` 12/12 passed；`EmbedDebug` 构建 passed；service include 边界检查无越界命中；`EmbedDebug.bat` exit=0 |
| commit | 待处理 |
