# ADR-0177：Header 三态响应式控制带

## 状态

已接受（ARCH-126 / UI-1.199，2026-08-12）

## 背景

现有 Header 在 546–900px 虽然没有水平滚动，但 status cluster 与动效/主题控制卡共用一条横排，导致连接上下文、数据来源和连接状态被
`BoundedTextLabel` 省略为 `UA…/实…/…`。这会直接损害串口调试时最重要的状态可读性。

## 决策

- 新增 `ResponsiveHeaderControls` 作为三个既有 header QFrame 的唯一几何 owner。它不拥有状态、信号、偏好、业务或动效时钟。
- 使用一个 `QGridLayout(SetNoConstraint)` 和三个 sizing-driven mode：REGULAR 三卡同行；COMPACT status 单独一行、motion/theme 同行；NARROW 三行。
- 三卡由 workspace 在注入时一次性置于 owner 的同一 QObject parent；模式切换只在同一 grid 内批量 remove/add，不跨 layout、不 setParent、不重建、不重连。
- mode 阈值使用 preferred sizing contract，`minimumSizeHint` 宽度使用 narrow hard-min contract；REGULAR 第一列保留 status preferred width，motion/theme 保留自身
  preferred width，避免宽屏等比 stretch 反向压缩状态文案。
- `_sync_header_density` 仍负责旧有 density/装饰件策略，完成内部 spacing/state caption 显隐后通知 owner invalidate；它不再拥有三卡的行列逻辑。

## 边界

本 ADR 只覆盖 Python/PySide6 presentation geometry；不改变 HeaderChromeBindings、lifecycle、theme tokens、MotionController、Tab/focus/accessibility、
UART/TCP/UDP/BLE/RTT 业务或 OTA/AES/RTT/J-Link contract-only/attach-only 边界。本轮无 embedded C/C++、固件或硬件改动，public-vendor-source applicability=N/A。

## 验证

三主题、`546/547/560/640/768/900/1120/1180/1240px`、状态完整文本、事件/焦点/往返 resize、页面 hmax=0、hide/show/close 均已通过；真实 Windows
GUI/HIDPI、实际显示器刷新率、EXE startup、硬件连接和正式发行资格仍未验证。
