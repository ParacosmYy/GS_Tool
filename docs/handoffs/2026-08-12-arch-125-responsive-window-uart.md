# ARCH-125 / UI-1.198 交接：窄窗口连接带与 UART 参数表单

日期：2026-08-12  
范围：Python/PySide6 presentation-only；embedded C/C++ public-vendor-source applicability=N/A。

## 结果

- 删除 `bootstrap.py` 的 `setMinimumSize(980, 720)`；保留 `resize(1240, 820)` 初始偏好，窄窗口由内容 sizing contract 自然收缩。
- `connection_builder.py::_ResponsiveConnectionBand` 负责既有连接控件的 `REGULAR/COMPACT/NARROW_COMPACT` 三态布局；窄态将连接、预设、
  保存/删除、context 与 status rail 分行。
- 新增 `presentation/responsive_uart_form.py::ResponsiveUartForm`，只接收十个既有 labeled field wrapper，负责 UART `REGULAR/COMPACT/NARROW`
  三态布局；不复制业务状态，不新增 raw control、timer、scroll owner 或 binding。
- FontChange、StyleChange、LayoutRequest、resize 均有 sizing invalidation；最后清理了构造阶段会被 owner 覆盖的无效 stretch，并简化 UART mode fallback。

## 评审记录

- 架构师：`019ff5c9-8eff-7cd3-9c2f-ab0a40ec9ef0`，owner/boundary/Qt sizing/简化均 `APPROVE`。
- 独立代码审查：`019ff5df-ef65-7f00-9b30-03c62fad4be9`，最终 `APPROVE WITH ADVISORIES`；Critical=0，Required=0。建议真实 Windows/HIDPI
  观察 style/frame width，并同步本交接文档，已完成文档同步。
- 独立简化审查：`019ff5df-efac-7453-96b5-f9d61b07c0ff`，最终 `APPROVE WITH ADVISORIES`；无必须简化项。未合并跨 owner 抽象，避免削弱
  connection/UART sizing contract；已采纳固定 NARROW fallback 的微简化。
- 额外独立终审线程：`019ff60b-f9d7-7d31-a87d-e91274e3ca6a` 在两次限定等待窗口内未返回，未计为通过；不伪造其 verdict。前两份独立最终审查
  已覆盖同一源码边界与最后微简化。

## 非破坏验证

- `python -m compileall -q src`：pass。
- `.venv\Scripts\ruff.exe check src`：pass。
- `scripts/check.ps1`：pass；source-limit 为 184 个 Python 文件均 `<=1000` 行，theme audit 为 3 themes / 22 semantic tokens / 19 selectors / 0 legacy literals。
- Qt offscreen 三主题矩阵：`520/546/560/640/768/900/980/1180px` 请求宽度下连接页、协议页、命令页、扩展页水平滚动条 `hmax=0`；FontChange、
  StyleChange、LayoutRequest、Fusion style、反复 resize 与生命周期均 pass。
- onefile 使用 `local-arch-125` 构建并通过 provenance verify；canonical/root/root-latest 均为 `48,057,767` bytes，SHA-256 为
  `C8F19055CA977477BA33C087E3CEEA92ACD99F328A2F7F2410189DF96D748BBA`；archive listing SHA-256 为
  `0642B8F37772873BF7D7DB9590125ACA77B319F3E06B2308BBE86826951C538F`；签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

## 未验证与残余风险

- 尚未在真实 Windows HIDPI、多显示器字体变更、真实 120Hz 显示器上做可见窗口验收；offscreen scheduler/geometry 证据不能外推为显示器实际 120fps。
- 未连接真实 UART/TCP/UDP/BLE/RTT/J-Link 硬件，未刷写、部署或操作目标设备。
- EXE 可打包与 provenance 可验证不等于签名或正式发行资格；签名、`release_eligible` 和硬件验收仍按发布门禁记录。

## 研发门禁记录

本轮没有 embedded C/C++ 源码变化，因此 public vendor source applicability=N/A；仍完成独立代码审查、行为保持简化评估和授权的非破坏静态/Qt offscreen
验证，不声称 MISRA、ISO 26262、ASPICE、ASIL 或其他认证合规。
