# ADR 0142：ViewModel worker owner 拆分

- 状态：Accepted for ARCH-91 / UI-1.164
- 日期：2026-08-12

## 背景

`presentation/viewmodels.py` 同时包含 Session facade、状态 projection、生命周期协调和四个 Qt
worker carrier/job。文件已接近 1000 行；继续放入新的 discovery worker 会模糊业务状态 owner，
也会提高后续 OTA/debug 扩展误接入 ViewModel 的风险。

## 决策

新增 `presentation/viewmodel_jobs.py` 作为 bounded worker module，迁移：

- `_DiscoverySignals` 与 `_DiscoveryJob`；
- `_BleScanSignals` 与 `_BleScanJob`。

新模块只负责 blocking discovery call、queued result signal、取消后的 emission gate 和异常到
`ErrorInfo` 的映射。`SessionViewModel` 保留全局 `QThreadPool`、两个 cancellation `Event`、
busy/closing 状态、status/error projection、回调连接、30 个 Qt signal 和所有公开方法。

## 依赖与生命周期

`viewmodel_jobs.py` 只能向内依赖 domain port/model/error 和 presentation Qt adapter，不得导入
`SessionViewModel`、controller、window 或 transport lifecycle。job 仍由 `SessionViewModel` 创建、
连接 signal 后提交到既有 `QThreadPool.globalInstance()`；没有新增线程池、timer、状态源或事件总线。

## 结果与验证

拆分后 `viewmodels.py=918`、`viewmodel_jobs.py=94`，均低于 1000 行。父代理完成依赖方向、行为
保持、简化和资源边界 fresh-pass；架构师和独立 review 调用均在服务窗口内超时并关闭，未伪造外部
PASS。Ruff、compileall、`scripts/check.ps1`、30-signal/2-job contract、source-limit、theme audit
和 provenance verify 通过；未启动 GUI/EXE、未采样显示器 FPS、未执行硬件/HIL 或签名验收。

交付 onefile：canonical、root、root-latest 均为 `48,007,307` bytes，SHA-256
`ED765A8442C2DAB1FCD11AC36FC78E53032EDCD7CC17E90EBAB96915C932932E`，source revision
`local-arch-91`，archive listing SHA-256
`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，签名 `NotSigned`，
`release_eligible=false`，`hardware_acceptance=not_run`。
