# PRD-130 Specs - Serial Station 测量趋势缓冲

## 执行边界

本轮只改 Serial Station 测量观察路径：

- `src/apps/serial_station/services/SerialMeasurementService.*`
- `src/apps/serial_station/services/SerialMeasurementExportService.*`
- `src/apps/serial_station/ui/SerialMeasurementPanel.*`
- `src/apps/serial_station/SerialStationController.*`
- `src/apps/serial_station/SerialStationWindow.*`
- 对应 `tests/serial_station/` 和 CMake 测试注册
- README 与评分追踪

不改协议帧格式、不改串口核心层、不改启动脚本。

## 架构准入

1. 主模块：`src/apps/serial_station/`。
2. 对外接口：service 值对象、Controller Qt signals、UI slot。
3. 依赖方向：`ui -> controller -> services/protocols`，service 不触碰 QWidget。
4. 装配点：`SerialStationWindow` 只连 signal/slot。
5. 验证：service QTest、panel QTest、controller QTest、workbench QTest、启动探针。
6. 状态：`E5/U3/D1` 保持，不提升设备状态。

## 设计

- 新增 `SerialMeasurementFrame`：保存 `frameIndex` 和该帧各通道 `values`。
- `SerialMeasurementSnapshot` 增加 `historyCapacity` 和 `recentFrames`。
- `SerialMeasurementService` 增加：
  - `setHistoryCapacity(int capacity)`
  - `QStringList trendLines() const`
  - `QString formatCsv() const`
- 新增 `SerialMeasurementExportService`：
  - 输入 `SerialMeasurementSnapshot`
  - 输出 CSV 文本或原子写入 CSV 文件
  - 不接触 QWidget、串口线程或协议解析
- `SerialMeasurementPanel` 增加只读趋势显示控件：
  - `serialMeasurementTrendLabel`
  - `serialMeasurementTrendView`
- `SerialStationController` 增加信号：
  - `serialMeasurementTrendUpdated(const QStringList& lines)`

## TDD 验收命令

```powershell
cmake --build build --target test_serial_measurement_service test_serial_measurement_export_service test_serial_measurement_panel test_serial_station_controller test_serial_station_workbench --parallel 4
.\build\tests\test_serial_measurement_service.exe
.\build\tests\test_serial_measurement_export_service.exe
.\build\tests\test_serial_measurement_panel.exe
.\build\tests\test_serial_station_controller.exe
.\build\tests\test_serial_station_workbench.exe
```

## 收口验证

```powershell
cmake --build build --target EmbedDebug test_serial_measurement_service test_serial_measurement_export_service test_serial_measurement_panel test_serial_station_controller test_serial_station_workbench test_startup_options --parallel 4
.\build\tests\test_serial_measurement_service.exe
.\build\tests\test_serial_measurement_export_service.exe
.\build\tests\test_serial_measurement_panel.exe
.\build\tests\test_serial_station_controller.exe
.\build\tests\test_serial_station_workbench.exe
.\build\tests\test_startup_options.exe
.\EmbedDebug.bat --station serial
```
