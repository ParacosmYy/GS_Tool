# ARCH-91 / UI-1.164 ViewModel worker owner 拆分

日期：2026-08-12  
范围：presentation SessionViewModel worker ownership。  
状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH91_VM_CONTRACT_PASS signals=30 jobs=2`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。

## 实施

- 新增 `src/serialforge/presentation/viewmodel_jobs.py`，唯一负责 `_DiscoveryJob`、`_BleScanJob`
  及其 signal carriers 的 blocking discovery、取消后的 emission gate 和错误映射。
- `src/serialforge/presentation/viewmodels.py` 保留 Session facade、全局线程池、两个取消 Event、
  busy/closing 状态、30 个 Qt signal、状态/错误 projection、回调和公开方法。
- `refresh_ports()`、`scan_ble()` 仍由 ViewModel 创建 job、连接 completed/failed signal 并提交到
  `QThreadPool.globalInstance()`；没有新增 timer、线程池、事件总线或状态源。
- 源码行数：`viewmodels.py=918`，`viewmodel_jobs.py=94`；均低于 1000 行。

## 架构与审查

本轮调用 Luna/max 架构师进行 import-format 边界复核；调用在服务窗口内超时并关闭，不能记作
架构师 PASS。独立只读 review 调用同样超时并关闭，未伪造独立 PASS。父代理完成 fresh-pass：确认
worker 行为、signal 数量、线程池归属、cancel Event、stop 生命周期、依赖方向和异常映射保持，
无 Required finding。嵌入式 C/C++/MCU/SDK/RTOS public-source applicability 为 N/A，本轮只修改
Python/PySide6 presentation，不作固件或认证合规声明。

## 已授权验证

- `uv run ruff check src scripts`：通过。
- `uv run python -m compileall -q src`：通过。
- `scripts/check.ps1`：通过，177 个源文件均不超过 1000 行，3 主题 token audit 通过。
- `ARCH91_VM_CONTRACT_PASS signals=30 jobs=2`：通过；ViewModel 仍绑定新模块中的两个 job。
- `uv run python scripts/provenance.py verify --manifest dist\\release\\0.1.0\\core\\onefile\\PROVENANCE.json`：通过。
- canonical/root/root-latest hash equality：通过。
- 未运行 GUI/EXE 启动、真实窗口几何/截图、显示器 FPS、HIDPI、硬件/HIL、连接/OTA/debug 和签名验收；
  原因是本轮没有新增 GUI 启动授权，且工程约束要求不把静态结果冒充运行时证据。

## 交付物

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
根目录：[SerialForge.exe](../../SerialForge.exe)；[SerialForge-latest.exe](../../SerialForge-latest.exe)  
三者大小：`48,007,307` bytes  
三者 SHA-256：`ED765A8442C2DAB1FCD11AC36FC78E53032EDCD7CC17E90EBAB96915C932932E`  
source revision：`local-arch-91`  
archive listing SHA-256：`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`  
签名：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`。
