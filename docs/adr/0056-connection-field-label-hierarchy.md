# ADR 0056：连接表单字段标签统一主题层级

日期：2026-08-10  
状态：已接受  
增量：UI-1.69

## 背景

连接配置页包含 UART、网络、BLE 三类表单。普通字段标签此前由多个裸 `QLabel` 直接创建，虽然继承了全局文字色，
但在三套主题和不同信息密度下与 section 标题、hint、状态摘要的层级不够稳定，也容易让后续维护者遗漏主题语义。

## 决策

- 在 `controllers/connection_builder.py` 内增加局部 `_field_label(text)` helper，统一创建 `QLabel` 并设置
  `role="muted"`。
- 仅将普通字段标签迁移到 helper；UART/网络/BLE 的 section 标题保留 `role="section"`，网络/RTT/BLE hint、
  BLE 属性摘要、连接状态和快捷提示保留自身角色与无障碍契约。
- helper 只属于 connection builder，不创建跨页面工厂、不读取 ViewModel、不接 signal、不写配置、不创建 timer，
  因而不会改变 controller 边界、焦点路径或业务状态。

## 被拒绝的方案

- 不为每个字段新增独立 QWidget，避免增加布局节点和 lifecycle 负担。
- 不用全局 `QLabel` 规则覆盖所有页面，避免压低 section、空态、状态和错误文案的视觉语义。
- 不把标签颜色写死在 builder，主题颜色继续由现有 `QLabel[role="muted"]` base/variant stylesheet 管理。

## 验证

- `scripts/check.ps1`：通过，147 个源码文件均不超过 1000 行，3 套主题 token audit 通过。
- `python -m compileall -q src`：通过。
- Qt offscreen 连接页 vector：三主题均发现 37 个 `role=muted` 字段/辅助标签、3 个 `role=section` 标题，
  preset/transport/BLE/network wiring 未改变。
- 未启动完整 GUI 或 EXE；HIDPI、读屏、真实硬件和正式发行验收需授权环境执行。
