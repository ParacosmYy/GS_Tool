# ADR-0088：连接页选项 affordance 与 presentation contract

日期：2026-08-11  
状态：Accepted  
范围：网络、RTT、TCP Server、BLE connection panel

## 背景

连接页的数值控件已经有明确的范围，但部分控件缺少上下文说明，RTT/BLE 写入模式仍暴露英文，
超时的零值状态显示为 `None`。用户需要通过选择和即时提示理解配置，而不是猜测字段或手填协议术语。

## 决策

- `connection_builder.py` 继续作为 presentation-only owner：负责可读 label、tooltip、accessible description 和 bounded selector affordance。
- 网络端口、客户端数、UDP 报文大小和超时仍使用既有 `QSpinBox/QDoubleSpinBox` 范围；不新增第二套 option catalog，也不改变 runtime 读取路径。
- RTT 通道和 BLE 写入模式显式保持不可编辑，visible label 本地化；`itemData(Qt.UserRole)` 的整数与 `BleGattWriteMode` string contract 保持不变。
- TCP Server allowlist、LAN 确认、BLE 过滤/缓存/配对等控件补齐上下文提示，但不新增连接动作、网络探测、vendor 工具启动或业务状态。
- `timeout_spin()` 只将零值特殊显示本地化为“未设置”，并保留原有 zero-as-unbounded 语义。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI198_CONNECTION_OPTIONS_VECTOR_PASS themes=3 rtt_localized=1 ble_write_localized=1 typed_data=1 none_localized=1 affordances=16
python scripts/provenance.py verify --manifest ...    PASS
```

向量使用真实组合根的 Qt offscreen 内存对象且未显示主窗口；Qt 仅报告 PySide6 环境缺少 fonts 目录的 warning。
未连接串口、未扫描 BLE、未启动 EXE 或硬件，未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
本轮没有嵌入式 C/C++ 变更，MCU vendor source applicability=N/A。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
