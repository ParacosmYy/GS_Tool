# ADR 0123：工作区滚动可发现性提示

状态：Accepted（UI-1.136）  
日期：2026-08-11

## 背景

工作区配置页已经统一使用 `controllers/composition.py:scroll_page()`，在
980×680 和 1180×780 窗口下通过纵向滚动保留完整配置。但首屏被压缩时，用户只能
从窄滚动条推断下方仍有内容，页面层级和可发现性不足。

## 决策

新增 `presentation/workspace_scroll_hint.py:WorkspaceScrollHint`。它只观察当前
`QScrollArea` 的原生 vertical scrollbar，并将其映射为四个 presentation 状态：顶部
“↓ 向下查看”、中部“↕ 上下滚动”、底部“↑ 返回顶部”和无溢出时“内容已全部显示”。
提示放入既有 `workspaceRouteStrip`，由 `workspace.py` 组装，
`workspace_runtime.on_workspace_tab_changed()` 在 Tab 切换时绑定当前页。

## 边界

- 不新增 application/domain DTO、业务状态、导航模型、设备 I/O、timer 或依赖；
- `scroll_page()` 仍是所有设置页的唯一滚动包装入口，横向滚动策略不变；
- 旧滚动条与原生键盘/焦点语义保持不变，提示为 NoFocus、鼠标透明且提供动态
  AccessibleDescription/tooltip；
- 状态颜色只复用既有 semantic token，base/variant QSS 对称覆盖；
- 旧 Tab 文案、路线 beacon、专注设置按钮和 MotionController 生命周期不改变。

## 验证

三主题 × 四工作区 × 1180×780 真实组合根离屏验证通过：顶部/底部状态切换正确，
vertical range 分别为 247/1052/103/790，主题截图 exact-white=0；compileall、Ruff
和源码行数门禁通过。GUI/EXE 启动、读屏、硬件与正式签名验收仍不在本轮授权范围内。

架构师线程 `019fedc5-ee6c-7a43-8178-9ee97bb0559d` 在两个限定等待窗口内未返回，
已关闭；父代理按本 ADR 完成 owner、信号解绑、无障碍、响应式、主题 token、行为保持
与简化审查。
