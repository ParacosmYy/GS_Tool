# ADR-0107：扩展工具站能力卡的静态 affordance 边界

日期：2026-08-11  
状态：Accepted  
范围：SerialForge presentation / embedded extension station

## 背景

扩展 / 工具站页面已经展示 OTA 传输、安全升级和 RTT/J-Link 的 contract-only / attach-only
能力，但能力卡在视觉上只是静态说明牌。需要提升扫描层级，同时避免让用户误以为这些预留能力已经
可以点击激活。

## 决策

- 保持 `embedded_extension_panel.py` 为卡片组装 owner；卡片仍是无焦点、无动作的 `QFrame`。
- 在 base 和 theme variant stylesheet 中增加轻量 `:hover` 边界：普通卡使用蓝色 signal，
  contract-only 使用 info/blue，attach-only 使用 history/purple。
- 不改变卡片的背景语义、文字、DTO、AccessibleName/Description 或只读能力状态。
- 两列 `QGridLayout` 使用等权 column stretch，避免宽度变化下卡片比例不稳定。
- 页面过渡继续复用 workspace 既有 180ms opacity transition；不为卡片创建局部 timer、第二套
  MotionController 或 hover 动画状态。

## 验证

真实组合根已验证 980/1180、三主题、四工作区切换、7 张卡片、vertical scroll range、
horizontal range=0、NoFocus 和三条 hover selector；中文字体使用生产入口的
`configure_application_font()`。截图已人工查看，未见白色背景带或主题脱落。

架构师线程在限定窗口内超时，未计为独立通过；父代理完成 owner、只读语义、token 对称性、
焦点/无障碍和简化审查。未修改嵌入式 C/C++；embedded applicability=N/A。
