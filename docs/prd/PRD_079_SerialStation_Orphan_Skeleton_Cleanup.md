# PRD-079 - Serial Station Orphan Skeleton Cleanup

## 背景

`src/apps/serial_station/` 同时存在两组入口、配置、常量、模型和窗口文件：

- canonical 版本：`SerialStationApp.*`、`SerialStationConfig.*`、`SerialStationConstants.h`、`SerialStationModels.h`、`SerialStationWindow.*`
- 小写残留版本：`app.*`、`config.h`、`constants.h`、`models.h`、`window.h`

小写残留版本未加入 CMake，且与 canonical 版本定义重复概念，会误导后续 UART 配置和 UI 开发。

## 目标

1. 删除未被 CMake 引用的小写残留骨架文件。
2. 保留 canonical `SerialStation*` 文件作为唯一落点。
3. 不修改 Serial Station 运行逻辑。
4. 不影响 `EmbedDebug.bat` 启动链路。

## 非目标

1. 不新增 UART 配置面板。
2. 不删除 `src/utils/`。
3. 不移动 Serial Station 目录。
4. 不改变旧主线 `src/serial/config/` UART 配置能力。

## 验收标准

1. 小写残留文件不再存在。
2. Serial Station QTest 继续通过。
3. 主目标构建不需要额外改动。
4. `EmbedDebug.bat` 仍可启动。

