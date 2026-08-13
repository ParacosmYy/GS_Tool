# ARCH-126 / UI-1.199 交接：Header 三态响应式控制带

日期：2026-08-12  
范围：Python/PySide6 presentation-only；embedded C/C++ public-vendor-source applicability=N/A。

## 结果

- 新增 `ResponsiveHeaderControls`，把 Header 的 status/motion/theme 三个既有 cluster 从单行拥挤布局收敛为 sizing-driven `REGULAR/COMPACT/NARROW` 三态。
- 546–560px 三行；640–900px status 独占一行、motion/theme 同排；1120–1240px 三卡同行。REGULAR 状态列优先保留 preferred width，避免宽屏重新省略。
- 三个既有 QFrame 在组合根完成创建与业务接线后一次性注入同一 owner；注入完成后 parent identity 保持稳定。模式切换只在同一 grid 内
  remove/add，未重建、跨 layout、业务侧重复 setParent 或重连 signals。
- `_sync_header_density` 只保留 density、装饰件显隐、state caption 和内部间距策略，并在最后触发 owner sizing invalidation。

## 架构裁决

- 架构师：`019ff613-3aea-7f92-b6db-2567ca74cee7`，初始 REQUEST CHANGES 后补齐 reparent、完整文本和高度契约，最终 `APPROVE`。
- 独立代码审查：`019ff62a-986e-71c0-98f0-13007776a724`，以及针对 `showEvent` 修复的复核
  `019ff62a-3bae-7cd2-a4c2-c82732a8110a`，均为 `APPROVE WITH ADVISORIES`（Critical/Required=0）。
- 简化评估：`019ff62a-98bb-7851-a19d-56c59142fded`，`APPROVE WITH ADVISORIES`，无必须简化项；保留单 child
  `controls_row.setSpacing(8)` 和 builder 注入前 parent=None 两条非阻断 advisory。

## 非破坏验证

- `python -m compileall -q src`：pass。
- `.venv\Scripts\ruff.exe check src`：pass。
- `scripts/check.ps1`：pass；source-limit 为 185 个 Python 文件均 `<=1000` 行，theme audit 为 3 themes / 22 semantic tokens / 19 selectors / 0 legacy literals。
- 三主题 × `546/547/560/640/768/900/1120/1180/1240px`：mode、parent identity、status 文本完整、四页 hmax=0 通过。
- FontChange、StyleChange、LayoutRequest、Polish、focus、`900→546→900`、hide/show/close 通过；视觉截图已人工检查三主题的 546/640/1180。

## 未验证与残余风险

- 未在真实 Windows 可见窗口、HIDPI、多显示器字体、实际 120Hz 显示器、EXE startup 或真实 UART/TCP/UDP/BLE/RTT/J-Link 上验收；未刷写、部署或操作硬件。
- 本轮 onefile 已使用 `local-arch-126` 重建并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical/root/root-latest
  均为 `48,064,034` bytes，SHA-256 为 `B55EF7C6BF173E5EEDB13CBD1F5355DA7249F0592684FABBEC85D85C3B7D6D91`，
  archive listing SHA-256 为 `489EE0ADAE68F2A57212C6A7776E7BF16E73598425936D81B45768C90BC0FDDA`，provenance verify pass。
  构建状态为 engineering build、签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
