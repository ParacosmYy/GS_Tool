# ARCH-95 / UI-1.168 交接归档：超时选项化

日期：2026-08-12  
范围：presentation 超时控件与连接 runtime binding；不涉及固件、设备 I/O 或硬件验收。

## 变更

新增 `presentation/bounded_value_combo.py` 的 `BoundedFloatCombo` 和统一
`TIMEOUT_OPTION_VALUES`；`controllers/composition.py` 新增 `timeout_combo()` 作为唯一构造入口。
UART、TCP/UDP/RTT、BLE 共八个超时控件从 `QDoubleSpinBox` 改为不可编辑下拉选项，显示 `ms/s`，
内部值仍是秒。UART 字节间超时的 `0` 继续显示“未设置”并映射为既有 `None` 语义。

`connection_bindings.py` 更新为具体的 bounded combo 类型，`connection_runtime.py` 保持原有
`.value()` 读取和 transport DTO 构造；默认值、最小/最大范围、禁用状态和连接 gate 不变。程序化
的超界值会裁剪，非选项旧值会被放进一个不可编辑临时 item，避免静默改变配置。

## 架构与复核

控件只属于 presentation leaf，不读取业务状态、不持久化、不创建 timer/线程，不依赖 transport、
OTA/AES、RTT/J-Link 或事件总线；runtime 仍是唯一的秒数解释 owner。所有相关文件均低于 1000 行。
架构师 `019ff31a-b844-7ca1-b77d-3d75984a90bc` 与独立 reviewer
`019ff320-947e-7850-bba9-ea631f12335a` 在服务窗口内超时并关闭，未形成外部结论；父代理完成
correctness、readability、architecture、security、performance 五轴复核与行为保持简化评估。
本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability 为
N/A，不作固件或认证声明。

## 验证

- `uv run ruff check src scripts`：通过。
- `uv run python -m compileall -q src`：通过。
- `scripts/check.ps1`：通过，178 files ≤1000，3 themes/22 semantic tokens，legacy QSS literals 0。
- 真实组合根 `QT_QPA_PLATFORM=offscreen`：8 个控件均 `editable=False`；option counts
  `[10, 11, 12, 16, 16, 16, 16, 16]`；默认值
  `[2.0, 1.0, 0.0, 3.0, 0.2, 3.0, 5.0, 30.0]`；超界裁剪、`0.123` 秒程序化值、runtime 读取
  通过；980×720 首屏无新增横向挤压。
- 当前 PySide6 环境打印缺失 font directory warning；没有将其误判为主题或布局失败。
- 未运行：GUI/EXE 启动、真实显示器/HIDPI/FPS、读屏、UART/网络/BLE/RTT 实连、OTA/debug 硬件、
  签名和正式 release acceptance。

## 交付

`local-arch-95` onefile 已生成并覆盖 canonical、root、root-latest；三者均为 `48,011,368` bytes，
SHA-256 为 `3CDC74EDDE5ED914E76436972FE4FB466A0AC754575F12D834A393C8C05D2E2E`；archive listing
SHA-256 为 `39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`；provenance verify
通过，签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。本轮未启动 EXE。
