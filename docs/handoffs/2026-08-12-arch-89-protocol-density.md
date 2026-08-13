# ARCH-89 / UI-1.162 协议/遥测页密度收敛

日期：2026-08-12  
范围：协议配置、组件遥测、Dataset/曲线、历史回放 presentation layout。  
状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH89_CONTRACT_PASS fields=37 detail_layout=QVBoxLayout`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。

## 实施

- 保留 `controllers/protocol.py:build_protocol_panel()` 和完整 `ProtocolPanelWidgets` 37 字段边界，
  未改变 callbacks、ViewModel、业务 controller、状态投影或 Tab/accessibility 契约。
- `protocol_detail_layout` 从 7 列 `QGridLayout` 改成 `QVBoxLayout` + 三个 `build_labeled_field`
  peer fields，说明文案独立占行。
- Dataset 配置状态与加载/导出动作分行；Replay 速度与选择/暂停/停止动作分行。
- root surface gap 从 4 调整为 12，单 surface 内部 spacing 从 6 调整为 10；没有新增 timer、
  thread、backend、scroll owner 或 motion clock。

## 架构与审查

本轮按要求调用 Luna/max 架构师只读审查，三个等待窗口均未返回，已关闭，不能记作架构师 PASS。
随后独立只读 review 调用同样在服务窗口内超时并关闭，未伪造独立 PASS。父代理完成 fresh-pass：
确认布局只触达组合 owner、37 字段仍完整、`composition.py`/runtime controller 依赖方向不变、
没有新增业务状态源或动画时钟，未发现 Required finding。后续若协议页新增独立状态源，再拆 typed
sub-binding/builder；本轮新增 builder 会扩大迁移风险，因此保留单 owner。

## 已授权验证

- `uv run ruff check src scripts`：通过。
- `uv run python -m compileall -q src`：通过。
- `scripts/check.ps1`：通过，175 个源文件均不超过 1000 行，3 主题 token audit 通过。
- 37-field import contract：通过。
- `uv run python scripts/provenance.py verify --manifest dist\\release\\0.1.0\\core\\onefile\\PROVENANCE.json`：通过。
- 未运行 GUI/EXE 启动、真实窗口几何/截图、显示器 FPS、硬件/HIL、连接/OTA/debug 和签名验收；原因是
  本轮没有新增 GUI 启动授权，且工程约束要求不把静态结果冒充运行时证据。

## 交付物

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
根目录：[SerialForge.exe](../../SerialForge.exe)；[SerialForge-latest.exe](../../SerialForge-latest.exe)  
三者大小：`48,004,714` bytes  
三者 SHA-256：`1D08E351935E0CA828C5AB095CD0712042F3C5F1DE6F0BD291595D508A6D3A44`  
source revision：`local-arch-89`  
archive listing SHA-256：`A152062F579FF9CCA9D2421EFDC6FA5D2FA6918A0691BE46EEE359AE451A47FF`  
签名：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`。
