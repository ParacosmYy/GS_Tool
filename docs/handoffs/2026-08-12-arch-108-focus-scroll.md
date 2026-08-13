# ARCH-108 / UI-1.181 交接：扩展能力焦点滚动结算

日期：2026-08-12

## 结果

扩展工具站的能力卡在 Tab 聚焦或点击后，仍由现有组合 owner 展示只读详情；本轮补齐焦点可见性：
详情展示改变 content 高度时，先同步结算现有 scroll content，再用原生 `ensureWidgetVisible` 和
有界 viewport 几何校正，让当前卡片保持在可视区内。

没有新增 timer、singleShot、processEvents、scroll owner、业务状态、OTA/AES/RTT/J-Link 后端或设备 I/O。

## 证据

- `scripts/check.ps1`：pass；source limit `179 files <= 1000`，theme token audit pass，ruff pass。
- 真实 Qt offscreen：三主题 × `980×720`、`1240×820`；7 张卡逐一 Tab 聚焦，全部
  `visible=True`、`focus=True`；详情标题依次同步 XMODEM/YMODEM/TFTP/AES-256-GCM/AES-128-CCM/
  RTT 原始打印/J-Link Telnet。
- 980px 首卡最终 viewport geometry `319..462`，viewport height `475`；1240px 首卡最终
  `408..551`，viewport height `563`；横向 scrollbar maximum 始终为 `0`。
- 三主题的 scroll hint 在连续聚焦后均为 `中段 · ↕ 上下滚动`，没有裁切或横向滚动。
- 本轮没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only asset。

## 审查与边界

- 架构师调用在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。
- 独立 reviewer 调用在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。
- 父代理完成 correctness、architecture、security、performance、readability 五轴 review 与
  behavior-preserving simplification assessment；确认修复只在 focus/click presentation 路径执行，
  不进入 shared MotionController 120Hz frame loop。
- embedded C/C++ public-vendor-source applicability：N/A；本轮仅修改 Python/PySide6 presentation。

## 交付

本轮文档完成后使用 `local-arch-108` 打包，覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,020,617` bytes，SHA-256 为
`2300FFD431657EA96E67A43F1470199671883CAC9AA5FC711F0CA1AB6330A217`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
EXE startup、真实 Windows 可见窗口、高刷新显示器/HIDPI、高负载、硬件连接、OTA/RTT 实连、签名和
正式硬件验收仍未运行或未授权；当前签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
