# ARCH-110 / UI-1.183 交接：命令空态内容居中

日期：2026-08-12

## 结果

命令管理页没有批量命令时，空态画布仍保持原有高度，但 glyph、说明和“新建批量命令”CTA
不再贴在左侧；内容组现在在画布中水平、垂直形成稳定的视觉焦点，消除宽屏右侧无语义空白。

## 边界

- 仅修改 `src/serialforge/presentation/command_batch_empty_state.py`：横向 root layout 两侧
  增加 stretch，copy layout 改为自然宽度。
- 保留 `CommandBatchEmptyState` 的唯一 vertical stretch owner；不改 command controller、
  ViewModel、命令执行、结果表、滚动 owner、CTA signal、Tab 顺序或 accessibility contract。
- 不新增 timer、thread、paint loop、独立动画时钟、业务状态或 OTA/AES/RTT/J-Link 依赖；继续复用
  唯一 `MotionController`。

## 证据

- `scripts/check.ps1`：pass；source limit `179 files <= 1000`，theme token audit pass，ruff pass。
- 真实 Qt `980×720`、`1240×820`：内容 union 中心偏差 `1px`；空态 geometry 分别为 `916×310`、
  `1176×399`；horizontal/vertical scrollbar maximum 均为 `0`。
- 三主题：`star_trail`、`moonlit_ocean`、`sakura_night` 均中心偏差 `1px`，无横向溢出。
- 空态与 CTA accessibility 完整；隔离 `new_requested` signal 检查通过，CTA 保持 `StrongFocus`。
- 低动效、暂停、隐藏/恢复、关闭通过；共享时钟 `65` frames、均值 `8.261ms`、p95 `9.299ms`、
  有效约 `121.05Hz`，target `120Hz`、timer `8ms`。
- 本轮没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only asset。

## 审查

架构师调用 `019ff421-d518-7bd1-851c-4ce4d2bb0340` 与独立 reviewer 调用
`019ff425-0bbb-7291-af25-202a226c72bd` 均在等待窗口内超时关闭，随后关闭线程，未形成外部
结论，未伪造 PASS。父代理完成 correctness、architecture、security、performance、readability
五轴 review 与 behavior-preserving simplification assessment。

本轮没有嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、
ASIL、ASPICE 或认证声明。EXE startup、真实 Windows 可见窗口、高刷新显示器/HIDPI、高负载、硬件
连接、OTA/RTT 实连和签名验收仍需授权环境验证。

## 交付

`local-arch-110` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,019,942` bytes，SHA-256 为
`EFE27AA251E89A9D857CD0E6D8D11B914A9AB24C730F802601225DD3465145A4`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
