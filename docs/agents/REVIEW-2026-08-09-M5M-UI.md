# SerialForge M5m UI 接入复核

日期：2026-08-09  
范围：M5m realtime stream boundary 的 presentation 接入  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建或操作 worktree  
嵌入式 C/C++ applicability：N/A（本轮仅 Python/PySide6 与文档）

## 用户结果与边界

用户现在可以在 UART/TCP Client 的协议编辑区显式选择 MAVLink v1/v2 stream；在 UART 上显式
选择 Modbus RTU timed。选择预设仍只载入编辑区，点击“应用”才通过 ViewModel 重置 parser、
component 和 Dataset 派生状态。Modbus timing 从当前 UART 的波特率、数据位、校验和停止位派生，
UI 显示 t1.5/t3.5，并明确 host read gap 只是诊断证据，不是线缆逐字节时间戳。协议状态栏新增
incomplete、parser drop、gap boundary 和 resync；raw terminal、recorder、transport、TX 和
generation 链未改动。

本轮不做：Modbus 主从事务/TX、物理 per-byte timestamp、MAVLink 完整 dialect/签名认证、
断开时 partial 的新产品语义、UDP/TCP Server/BLE/RTT 协议扩展、动态二次元背景、真实硬件、
GUI offscreen 和正式发行授权。

## 六角色只读评审

| 角色 | agent id | 结果 | 关键发现 | 父代理动作 |
|---|---|---|---|---|
| 产品 | `019fe5e9-081c-7920-8e50-f45697be9a32` | revise/P0 | UI 缺少两个 framing、Modbus timing 和 incomplete/gap/resync 统计；要求保留 host-gap 限定与 raw truth。 | 已实现 presentation-only 接入；保留硬件/GUI 后置门。 |
| 架构 | `019fe5e9-0862-7eb0-9744-44e60db4455b` | pass to code | domain/application/generation 契约已就绪，最小改动限定在 MainWindow；断开 partial 丢弃不在本轮扩张。 | 只修改 `main_window.py` 的业务接入，未改 worker/port。 |
| UI 设计 | `019fe5e9-08a4-7a92-a614-eb8a7c39729e` | revise | 需显示只读 t1.5/t3.5、Host-gap 质量、固定 256/280 B 上限，并保持 preset 载入/应用分离。 | 已增加 timing hint、专用上限和明确 scope；未固化动态美术方向。 |
| 开发 | `019fe5e9-08e0-7830-8272-7b6ad8b41809` | pass to code | 只需更新 framing/editor/apply/scope/stats 相关 MainWindow 符号。 | 按最小文件范围实现。 |
| 验证 | `019fe5e9-091a-7f00-bcc7-68ebd86f59aa` | revise | 要求 locked compile/Ruff、MAVLink/Modbus 内存向量；host gap、硬件、GUI 和签名/方言仍不能宣称通过；另发现一处既有 format 问题。 | 修复该行为保持格式问题并重跑全部静态/向量门。 |
| 打包/流程 | `019fe5e9-095c-7003-aa8d-4f4e03441a9f` | revise | 需更新本轮 handoff 并重建四矩阵；无新依赖/资源，package.ps1 无需变化。 | 已重建 core/BLE × onedir/onefile 并新增本轮交接。 |

## 实际改动

- `src/serialforge/presentation/main_window.py`：新增 M5m framing 选项、UART timing 派生、
  timing/host-gap 提示、Modbus UART scope、协议固定帧上限、preset round-trip 和扩展统计展示。
- `src/serialforge/domain/stream_boundaries.py`：仅修复既有 Ruff format 换行，不改变条件或行为。
- `README.md`、`docs/ROADMAP.md`、`docs/PROTOCOLS.md`、`docs/WORKFLOW.md`、
  `docs/adr/0022-realtime-stream-boundary-m5m.md`：同步当前代码完成状态和后置验收边界。
- `docs/agents/REVIEW-2026-08-09-M5M-UI.md`：本轮评审与证据归档。

## 复核、简化与验证证据

- 架构复核确认未绕过 ViewModel，不直接访问 parser worker；Modbus timing 只从已有 UART 控件
  组装，不修改 immutable preset；MAVLink/Modbus 专用 framing 禁用通用 checksum，固定上限与
  domain decoder 一致。
- `uv run --locked --extra dev ruff format --check src scripts`：`53 files already formatted`。
- `.\scripts\check.ps1`：locked sync、compileall、Ruff check 均通过。
- 临时进程内 bytes/DTO 向量：Modbus 9600 8N1 的 t1.5/t3.5、t3.5 完整 ADU、t1.5–t3.5
  `INCOMPLETE`、无 timing quality 合并，以及 MAVLink 噪声重同步/`UNVERIFIED` 均通过；向量未落盘。
- AST/source assertions：确认 M5m framing、timing builder、apply 路径和新增统计字段均存在并可解析。
- `.\scripts\package.ps1 -Mode onedir`、`-Mode onefile`、`-Mode onedir -Ble`、
  `-Mode onefile -Ble`：四个产物均创建并由脚本 provenance verify；manifest 读回确认 core
  无 Bleak/WinRT、所有变体无 vendor binary、签名 `NotSigned`、`release_eligible=false`。

专门的独立 Luna 复核代理曾因超时未返回结论并被关闭；父代理随后完成了逐符号、契约和简化
审计，没有发现 P0/P1 行为问题。该事实不替代 GUI/硬件证据，也不把本轮标记为正式发行通过。

## 未运行与风险

1. 未启动 GUI/offscreen 或 EXE：用户和仓库约束禁止默认持续启动软件；因此未验证控件实际布局、
   焦点顺序、读屏播报、窗口关闭和运行时加载。
2. 未连接真实 UART、TCP、BLE、J-Link 或目标板：没有授权设备环境；host read gap 不能证明
   物理 t1.5/t3.5，MAVLink dialect/signature 和 Modbus 事务仍未验收。
3. 未运行 unit test、mock、fixture、harness 或测试专用资产：遵循项目规则；本轮仅使用临时
   进程内向量、静态检查、打包和 manifest 证据。
4. 包仍是未签名 engineering build，许可证仍是 inventory-only；不能宣称正式发行或认证合规。

## 下一步

1. 获得授权后执行 M5m Qt offscreen/GUI 门，重点检查 preset 应用、timing hint、固定上限、
   incomplete/gap/resync 状态和关闭生命周期。
2. 用户选择动态 UI 的视觉方向后，再以 presentation-only 小切片实现可暂停、reduced-motion、
   资源失败回退的背景，不把动画状态接入业务。
3. 按路线进入下一个未交付能力；真实硬件验收、Modbus transaction、MAVLink dialect/signing、
   RTT 和正式许可证/签名门继续保持独立授权边界。

结论：M5m presentation 代码切片和工程交付包已完成并有静态/内存/打包证据；GUI、硬件和正式
发行仍是明确的后置门，下一位协作者从根 `handoff.md` 继续。
