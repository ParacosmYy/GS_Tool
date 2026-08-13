# ARCH-90 / UI-1.163 共享动效 owner 拆分

日期：2026-08-12  
范围：presentation shared motion lifecycle owner。  
状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH90_CONTRACT_PASS motion_reexports=3 target_hz=120 interval_ms=8`；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。

## 实施

- 新增 `src/serialforge/presentation/controllers/lifecycle_motion.py`，唯一负责 motion catalog、
  visibility snapshot、activity gate、phase-offset fan-out 和 live-RX activity projection。
- `lifecycle.py` 保留主题、状态、错误、connection context 和 Qt 生命周期；通过兼容导入继续提供
  `on_motion_frame`、`invalidate_motion_surface_snapshot`、`set_data_activity_motion` 与 stop helper。
- `workspace.py`、`terminal_runtime.py`、`workspace_runtime.py` 调用路径未变；未创建第二个 timer、
  scroll owner、线程、backend 或状态源。
- 源码行数：`lifecycle.py=662`，`lifecycle_motion.py=300`；均低于 1000 行。

## 架构与审查

本轮按要求调用 Luna/max 架构师只读审查，三个等待窗口未返回，已关闭，不能记作架构师 PASS；
独立只读 review 调用同样超时并关闭，未伪造独立 PASS。父代理完成 fresh-pass：确认 catalog
顺序、phase offset、activity-only stop、visible snapshot、show/hide/close/rearm、3 个兼容符号、
依赖方向和 120Hz 不变量均保持，无 Required finding。嵌入式 C/C++/MCU/SDK/RTOS public-source
applicability 为 N/A，本轮只修改 Python/PySide6 presentation，不作固件或认证合规声明。

## 已授权验证

- `uv run ruff check src scripts`：通过。
- `uv run python -m compileall -q src`：通过。
- `scripts/check.ps1`：通过，176 个源文件均不超过 1000 行，3 主题 token audit 通过。
- motion contract：通过；`lifecycle` 与 `lifecycle_motion` 的 3 个兼容符号 identity 保持，
  `MotionController.TARGET_HZ=120`、`_BASE_INTERVAL_MS=8`。
- `uv run python scripts/provenance.py verify --manifest dist\\release\\0.1.0\\core\\onefile\\PROVENANCE.json`：通过。
- canonical/root/root-latest hash equality：通过。
- 未运行 GUI/EXE 启动、真实窗口几何/截图、显示器 FPS、HIDPI、硬件/HIL、连接/OTA/debug 和签名验收；
  原因是本轮没有新增 GUI 启动授权，且工程约束要求不把静态结果冒充运行时证据。

## 交付物

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
根目录：[SerialForge.exe](../../SerialForge.exe)；[SerialForge-latest.exe](../../SerialForge-latest.exe)  
三者大小：`48,005,887` bytes  
三者 SHA-256：`E3EAE6FCC3CEB46C498EBB3094B70551211C80168BD54D327B4FCE178CD1EF9E`  
source revision：`local-arch-90`  
archive listing SHA-256：`0FC3A33559DC02B0C14313BE9A76FDE3C0FAA881A5C7861C9DFE146A0307775A`  
签名：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`。
