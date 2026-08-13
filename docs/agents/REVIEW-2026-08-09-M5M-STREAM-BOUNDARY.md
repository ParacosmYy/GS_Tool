# M5m realtime stream boundary 六角色记录

日期：2026-08-09  
范围：`domain.timing`、MAVLink/Modbus stream boundary、protocol ingress timing、queue
continuity epoch；UI 控件、真实硬件和 J-Link RTT 不在本轮写入范围。父代理是共享 checkout
唯一源码写入者；六角色只读复核；未创建、修改或运行测试专用资产。

## 六角色结果

| 角色 | 子代理 | 结果 | 关键结论 |
|---|---|---|---|
| 产品 | `019fe53e-1e33-7df1-9c0f-8617741aa4dc` | revise | UI 必须显式展示 incomplete/gap/resync；host read gap 只能标为诊断启发式；MAVLink 无 profile 不得显示 VALID。 |
| 架构 | `019fe53e-1e7a-7053-b361-bbf1ecc4ce2a` | revise | queue drop 必须形成 source continuity epoch；epoch 在 worker 出队处落地，不能提前 reset 造成旧队列项跨段拼接。 |
| UI 设计 | `019fe53e-1ec4-7aa1-9f0c-cb25abd4b296` | revise | 新边界统计需与业务帧状态分层呈现；动态背景必须可暂停并支持 reduced-motion；视觉实现等待用户选择方向。 |
| 开发 | `019fe53e-1f07-7842-9bb9-4bd4dee3344c` | pass with notes | DTO、MAVLink/MODBUS boundary 和 pipeline integration 保持 domain/application 分层；需补 UI preset/config 接入。 |
| 验证 | `019fe53e-1f54-77e2-bbc2-1ab2dc5f1ae7` | revise → pass after fix | 初始 Ruff 和 queue continuity 证据不足；修正后静态门、bytes vectors、坏长度重同步和 profile upgrade 通过。 |
| 打包/流程 | `019fe53e-1fb8-79e0-96b4-02ba8074466c` | revise | 文档必须说明 M5m 是后端条件完成；不引入运行时依赖、厂商 DLL 或 RTT 提前依赖。 |

六个角色均已返回并关闭；无只读代理继续占用共享 checkout。

## 公开来源适用性

- Modbus：Modbus Organization，`Modbus Serial Line Protocol and Implementation Guide V1.02`，
  公开版本，适用 Modbus Serial Line RTU framing；t1.5/t3.5 只作为 domain timing contract
  的工程依据，host read gap 不因此升级为物理线缆测量。
- MAVLink：项目已有官方 MAVLink overview、serialization、message signing 和 common.xml
  引用；这些是公开协议/实现参考，不是特定飞控制造商的内部要求。profile 的 dialect revision
  仍需用户在生产配置中固定。
- 嵌入式 assurance applicability：本轮只修改 Python desktop application，未修改 MCU、BSP、
  HAL、RTOS、ISR、driver 或 firmware C/C++；`$mcu`、`$embedded-enterprise-workflow` 和
  `$embedded-code-review-simplifier` 的嵌入式源码适用性为 N/A，不能推导 MISRA、ISO 26262、
  ASPICE、ASIL 或任何认证结论。

## 独立复核与简化评估

- 复核确认 raw terminal/recorder 仍是事实路径，boundary 只生成 bounded derived frames；
- `GapObservation` 将 gap 数值和质量绑定，避免一个裸 float 在 UI 或 codec 中被误读成 wire time；
- MAVLink profile validation 留在既有 component codec，stream decoder 只做有界结构提取和
  resync，避免复制 CRC_EXTRA/dialect/signing 逻辑；
- Modbus timing 只通过 optional `feed_unit()` 接口进入，旧 `StreamingFrameDecoder`、TCP/UDP/
  BLE/RTT transport 和普通 framing 不增加时序分支；
- queue drop 修复为单一 source epoch barrier，没有增加全局缓冲、插件系统、脚本或重型协议库；
- 仍存在明确后续项：MainWindow preset/config/stats 接入、断开时 partial 的 UI 语义、真实 UART
  per-byte timestamp、完整 Modbus transaction 和 MAVLink dialect/signature 语义。

## 授权、非破坏性验证证据

- `uv run --locked --extra dev python -m compileall -q src scripts`：通过；
- `uv run --locked --extra dev ruff check --no-cache src scripts`：通过；
- `uv run --locked --extra dev ruff format --check --no-cache src scripts`：通过；
- 临时进程内 vectors：MAVLink noise/partial、corrupt-length resync、无 profile `UNVERIFIED`、
  common heartbeat profile upgrade、Modbus t3.5 host-gap、无质量 gap 合并、t1.5～t3.5
  `INCOMPLETE`：通过；向量未写入仓库；
- 没有启动 GUI/EXE，没有创建/运行 unit test、mock、fixture 或 harness，没有连接或写入真实
  UART、BLE、Wi-Fi、TCP/UDP 设备，也没有启动/操作 J-Link/目标板。

## 未决风险

1. host read scheduling 和 USB/UART driver buffering 会让 gap 不能证明物理 t1.5/t3.5；真实
   RTU acceptance 必须使用经授权、可提供硬件/每字节 timing 证据的环境；
2. MAVLink stream resync 是保守结构启发式，异常 payload 中的 magic 仍可能增加延迟或保留为
   candidate；不能替代 dialect CRC_EXTRA 和 signing verifier；
3. disconnect/error 当前由 session bridge reset parser，部分尾帧的可见性仍需单独定义并接入 UI；
4. 当前 UI 仍未暴露两个新 framing 选项和四类新增统计，M5m 不能标成完整产品验收；
5. UI 动态背景尚未实现，等待用户从已生成的三个方向中选定一个；动画必须提供暂停、低动效
   和资源加载失败回退。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
