# ADR-0094：BLE GATT 连接配置密度与组合边界

日期：2026-08-12

状态：已采用

## 背景

BLE GATT 参数页把扫描、名称/Service 过滤、设备选择、连接策略、特征和收发动作放进单个 7 列网格。
在 980px 窗口中虽然没有几何溢出，但字段标签和动作挤在同一条视觉带，用户需要先理解网格位置才能
完成扫描到特征读写的顺序操作。

## 决策

- 新增 `presentation/controllers/ble_builder.py`，由 `build_ble_panel()` 负责 BLE surface 的全部
  presentation 组合；`connection_builder.py` 只接入它返回的既有 `BleControlBindings`。
- 按用户任务分为“扫描与筛选”“设备发现与连接”“GATT 特征与收发”，每个字段通过既有无状态
  `form_fields` helper 组合，纵向空间交给既有外层 settings scroll。
- 保持 `BleControlBindings`、`BleGattWriteMode` itemData、默认值、callbacks、focus/accessibility、
  不自动扫描/连接/重连边界和所有 backend 行为；不新增 timer、线程、设备 I/O、vendor SDK 或状态源。

## 结果

BLE 页由位置记忆改为任务顺序，字段拥有稳定的标签和可读间距；连接 shell 与 BLE form 的 owner 分离，
后续增加 GATT 发现/解析展示时不必继续膨胀多传输 builder。

## 验证

- `scripts/check.ps1`：通过；170 个源文件均不超过 1000 行，3 themes、22 semantic tokens、19 selectors。
- Qt offscreen：980×720/1240×820 × 三主题，field overlap=0、horizontal maximum=0、exact-white=0；
  `BleControlBindings`、写入模式 itemData、关键控件 focus、reduced-motion、close 清理通过。
- 视觉截图：`C:\Users\Gs\AppData\Local\Temp\serialforge_ble157_980.png`、`serialforge_ble157_1240.png`。
- 未触发扫描、连接、配对、通知、写入或任何真实硬件动作；未创建或运行 unit test、mock、fixture、
  harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本轮仅修改 Python/PySide6 presentation，没有 firmware/MCU/BSP/HAL/RTOS/
bootloader/Flash/OTA backend 修改，不声明厂商要求或认证合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
