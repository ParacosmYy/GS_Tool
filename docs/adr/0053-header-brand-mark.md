# ADR 0053：Header 资源无关星轨品牌徽记

## 状态

已接受（UI-1.66，2026-08-10）。

## 背景

SerialForge 已有三套深色二次元配色、顶部信号场、连接状态胶囊和 `SERIALFORGE` wordmark，但 Header 左侧仍缺少一个稳定
的视觉锚点。用户尚未指定角色、作品或具体 IP 美术方向，因此本轮不能擅自加入角色立绘、GIF、背景图或字体资源；需要一个
可替换、可审计、不会改变业务语义的品牌装饰。

## 决策

新增 `presentation/brand_mark_surface.py:BrandMarkSurface`，使用固定 38×38 的 `QPainter` 绘制星轨、节点和简化 S 形
几何徽记。颜色来自父窗口当前 `ThemeSpec`，不读取连接/协议/传输状态，也不创建资源加载器、timer 或外部依赖。

`controllers/workspace.py` 只负责把徽记放在现有 `SERIALFORGE` wordmark 前；`controllers/lifecycle.py` 通过已有
`_motion_surfaces()` 传递共享 `MotionController` frame/stop。徽记 `NoFocus`、鼠标透明、AccessibleName/Description 为空，
不会改变焦点顺序、读屏树、标题文案、状态胶囊、主题选择器或业务状态。

动效只表现为低干扰的轨道 halo/pulse；`stop()` 保留静态星轨和中心标记。现有 `apply_theme()` 对所有子 QWidget 调用
`update()`，因此主题切换不需要额外的 theme callback 或第二套刷新机制。

## 未采用的方案

- 角色立绘、作品 logo 或外部背景：用户没有选择具体 IP/素材方向，且会引入版权、打包、加载失败和高 DPI 资源治理。
- SVG/PNG/QRC 资源：固定小尺寸几何图形用 `QPainter` 已足够，不增加资源索引和发行包变体。
- 独立 `QTimer`：会破坏共享动效时钟和隐藏/暂停生命周期，现有 `MotionController` 已提供足够的帧频。
- 将连接状态嵌入徽记：会复制业务事实并让装饰取代 Header 的可访问文字状态。

## 影响与验证

正向影响是首屏获得稳定品牌锚点，同时保持当前主题、原生控件和高内聚 presentation 边界；性能成本是每个共享 frame 多一个
38×38 bounded paint consumer。组件文件保持在 200 行以内，未引入依赖和测试专用资产。

已通过 `UI166_BRAND_MARK_VECTOR_PASS`：三套主题中启用 frame 后渲染中心像素非透明，`stop()` 后保持静态，焦点策略为
`NoFocus`；Ruff、源码行数门禁、compileall 和导入检查通过。完整 GUI/offscreen/HIDPI/读屏/EXE 启动及真实设备验收未在本轮
授权范围内运行。
