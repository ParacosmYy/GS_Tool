# ARCH-120 / UI-1.193 工具栏 intrinsic sizing 交接

日期：2026-08-12  
范围：实时观测栏与发送栏的 presentation sizing policy；不涉及业务或数据路径。

## 结果

实时观测栏的清空/记录动作按钮恢复 intrinsic width；发送栏的发送模式、发送、CRLF、快捷命令和保存快捷
保持 intrinsic width，输入框、发送摘要和状态 rail 吸收剩余空间。980px 总览态不再把动作控件等权拉宽；
既有 `TerminalControlBindings`、signals/callbacks、focus/accessibility、接收/发送语义、唯一 120Hz MotionController
和 OTA/AES/RTT/J-Link 边界保持不变。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`TERMINAL_SEND_RESPONSIVE=pass`；`TERMINAL_SEND_THEME_LIFECYCLE=pass`（36 checks，0 failures）；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。

offscreen PySide6 font-directory warning 只影响截图字形；未新增或运行 unit test、mock、fixture、harness 或
test-only 资产。独立 reviewer 超时关闭，未形成外部结论。

## 架构与审查

架构师 `019ff51d-9728-75b2-9f96-c144beeec9b1` 批准工具栏双 owner；Terra `019ff523-c27e-7bd2-8c2c-c46c4b99527e`
批准发送栏 intrinsic sizing 并明确禁止 `setFixedWidth(sizeHint())`。父代理完成六轴 review 与简化评估；本轮
无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 交付产物

使用 `local-arch-120` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,038,525` bytes，SHA-256 为
`05A72BF2599736C65BA4B43CFB525F72FED4EB2C2CA4E47672BAC46B2E07165B`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名、
正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
