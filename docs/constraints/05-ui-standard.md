# 05 - UI执行标准

> 本文档是 EmbedDebug 约束体系的第5模块。所有UI相关开发必须遵守。

---

## 一、设计灵感

| 标杆产品 | 借鉴要点 |
|---------|---------|
| **Linear** | 极简暗色、微边框、精准留白、流畅面板切换 |
| **Raycast** | 毛玻璃效果、键盘优先、优雅弹出面板 |
| **Arc Browser** | 侧边栏彩色标签、丝滑Tab切换 |
| **Vercel仪表板** | 数据展示清晰、状态指示优雅 |
| **Warp终端** | 现代终端体验、命令块分组 |

**核心设计理念**: "克制而精致" — 每个动效都有明确的功能目的。

---

## 二、设计原则

| 原则 | 执行要求 |
|------|---------|
| **一致性** | 相同功能使用完全相同的视觉表现 |
| **层次分明** | 主操作区 > 辅助区 > 背景装饰，亮度递减 |
| **留白呼吸** | 最小间距 8px，分组间距 16px，面板内边距 12px |
| **即时反馈** | 按钮有hover/pressed状态、加载有指示、操作有通知 |
| **信息密度** | 关键数据实时可见，次要信息按需展开 |
| **动效克制** | 动画必须服务于功能 |

---

## 三、配色体系

所有主题必须定义以下语义色板:

```
--bg-primary      : 主背景（最深）
--bg-secondary    : 次背景（面板/卡片）
--bg-tertiary     : 三级背景（输入框/下拉）
--bg-hover        : 鼠标悬浮背景
--text-primary    : 主文字
--text-secondary  : 次文字
--text-muted      : 弱文字
--accent          : 强调色
--accent-hover    : 强调色悬停（亮10-15%）
--accent-pressed  : 强调色按下（暗10-15%）
--border          : 边框色
--border-focus    : 焦点边框色
--success         : 成功色
--warning         : 警告色
--error           : 错误色
--shadow          : 阴影色
--scrollbar       : 滚动条默认色
--scrollbar-hover : 滚动条悬停色
```

**禁止**: 在QSS中硬编码 `#RRGGBB` 色值用于语义功能。

---

## 四、布局与间距

| 元素 | 尺寸 |
|------|------|
| 工具栏高度 | 36-40px |
| 导航树宽度 | 180-280px |
| 面板内边距 | 8-12px |
| 控件间距 | 6-8px |
| 分组间距 | 12-16px |
| 按钮最小高度 | 28px |
| 输入框高度 | 28-32px |
| 发送区域高度 | 36-40px |
| 状态栏高度 | 24-28px |
| 搜索栏高度 | 32-36px |
| 圆角统一值 | 6px |

---

## 五、排版规范

| 规则 | 值 |
|------|-----|
| 终端字体 | Consolas / JetBrains Mono / Source Code Pro |
| 界面字体 | "Microsoft YaHei UI" / "Segoe UI" |
| 终端字号 | 13-14px |
| 界面字号 | 12-13px |
| 行高 | 字号 × 1.5 |

---

## 六、动画规范

所有动画使用 QPropertyAnimation，缓动曲线统一使用 QEasingCurve。

### 必须实现的动画

| 动画 | 场景 | 时长 | 缓动 |
|------|------|------|------|
| 面板滑入展开 | 切换面板 | 250ms | OutCubic |
| 面板滑出收起 | 替换面板 | 200ms | InCubic |
| 搜索栏展开 | Ctrl+F | 200ms | OutCubic |
| 搜索栏收起 | Esc | 150ms | InCubic |
| 连接状态脉冲 | 连接中 | 1500ms循环 | InOutSine |
| 进度条流动 | OTA传输 | 2000ms循环 | Linear |
| 进度条完成 | 100% | 400ms | OutCubic |
| 导航树选中滑动 | 点击导航项 | 250ms | OutCubic |
| 通知吐司弹出 | 操作完成 | 300ms | OutBack |
| 通知吐司消失 | 自动消失 | 250ms | InCubic |
| 主题切换过渡 | 切换主题 | 300ms | InOutCubic |

### 动画铁律

1. **使用 QPropertyAnimation** — 禁止用 QTimer 手动插值
2. **缓动曲线统一** — 展开用OutCubic，收起用InCubic
3. **时长限制** — 最短100ms，最长400ms
4. **禁止**: 弹跳效果(通知除外)、3D旋转、超过500ms的动画、彩虹色渐变
5. **性能** — 帧率不低于30fps

---

## 七、组件设计规范

### 按钮体系

| 类型 | 外观 | 场景 |
|------|------|------|
| 主要按钮 | 实心accent + 白字 | 连接、发送 |
| 次要按钮 | accent描边 + 透明背景 | 清除、导出 |
| 危险按钮 | 实心error + 白字 | 断开、取消 |
| 幽灵按钮 | 无边框 + text-secondary | 折叠、选项 |
| 禁用态 | opacity=40% | 条件不满足 |

### 输入框
- 正常: bg-tertiary + 1px border + 6px圆角
- 焦点: 2px border-focus + 轻微外发光
- 错误: 2px error边框 + 浅红背景

### 下拉框
- 与输入框等高同风格
- accent色下拉箭头
- 选中项: bg-hover背景

### 树形导航
- 选中: bg-hover + 左侧3px accent竖线（滑动动画）
- 悬浮: bg-hover半透明，无竖线
- 连接类型图标: 串口(蓝) TCP(绿) UDP(黄) RTT(紫)

### 状态栏
- 底部固定，24-28px高
- 左: 连接状态呼吸灯 | 中: 连接名 | 右: RX/TX字节+速率

### 通知吐司
- 右下角弹出，3秒自动消失
- 成功: success左边框 | 错误: error左边框

---

## 八、UI执行铁律（违反不允许commit）

1. **禁止C++中硬编码颜色到setStyleSheet()** — 颜色从QSS获取
2. **所有QWidget必须设置objectName** — QSS依赖
3. **新增控件必须在三主题QSS中同步添加**
4. **按钮必须有hover/pressed/disabled三种状态**
5. **所有用户可见文字必须用tr()包裹**
6. **面板切换必须有过渡动画**
7. **状态变化必须有视觉反馈**

---

## 九、观感评审检查清单

- [ ] 所有颜色使用语义色板，无硬编码
- [ ] 间距符合标准
- [ ] 字体使用正确
- [ ] 按钮有hover/pressed/disabled状态
- [ ] 输入框有normal/focus/error/disabled状态
- [ ] 连接状态有明确颜色 + 呼吸动画
- [ ] 导航树选中态清晰
- [ ] 面板切换有动画
- [ ] 搜索栏有展开/收起动画
- [ ] 主题切换有淡入淡出
- [ ] resize时布局不错乱
- [ ] 动画帧率≥30fps

---

## 十、BasePanel容器规范

BasePanel 是所有可切换面板的标准外壳组件, 提供统一的标题栏、折叠动画和视觉风格。

### 结构

```
┌────────────────────────────────────────┐
│ [icon 16px] 标题文字 (bold 13px)  [▼]  │  ← 标题栏 36px
├────────────────────────────────────────┤
│                                        │
│  内容区域 (padding: 12px top/sides,    │
│            12px bottom)                │
│                                        │
└────────────────────────────────────────┘
```

### 参数

| 属性 | 值 |
|------|-----|
| 标题栏高度 | 36px |
| 标题栏图标 | 16px |
| 标题文字 | bold, 13px, TextPrimary色 |
| 折叠按钮 | 24px × 24px, Ghost按钮风格 |
| 内容区上/侧边距 | 12px |
| 内容区下边距 | 12px |

### 折叠行为

- 点击折叠按钮: 内容区隐藏, 标题栏保留, 通过 height 属性动画过渡
- 展开动画: 250ms, OutCubic, fade+slide
- 收起动画: 200ms, InCubic, fade+slide
- 折叠状态: 标题栏显示展开图标(▶), 展开状态显示折叠图标(▼)

### 免包装面板

以下面板**不使用** BasePanel 包装:

| 面板 | 原因 |
|------|------|
| Terminal | 始终可见, 不参与面板切换 |
| SearchBar | 叠加层(overlay), 非面板 |
| QuickCmdBar | 底部固定栏, 非切换面板 |

---

## 十一、深度与标高系统

通过阴影层级区分UI元素的视觉层次, 所有层级通过 CSS 变量定义。

### 层级定义

```css
--elevation-0 : 无阴影 (flat, flush with surface)
--elevation-1 : 微阴影 (面板浮起, 1px blur)
--elevation-2 : 中阴影 (弹窗/弹出面板, 4px blur)
--elevation-3 : 重阴影 (模态对话框, 8px blur)
```

### 使用规则

| 组件 | 层级 | 说明 |
|------|------|------|
| BasePanel(默认) | elevation-1 | 面板从背景浮起 |
| 弹出面板(Popup) | elevation-2 | 下拉菜单、Popover |
| 模态对话框(Dialog) | elevation-3 | 确认框、设置弹窗 |
| 导航树 | elevation-0 | 与主背景齐平 |
| 工具栏 | elevation-0 | 与主背景齐平 |
| 搜索栏(overlay) | elevation-2 | 浮于终端之上 |
| Toast通知 | elevation-3 | 最顶层 |

### QSS示例

```css
/* elevation-1 */
QGroupBox[objectName="panel"] {
    box-shadow: 0 1px 3px var(--shadow);
}

/* elevation-2 */
QWidget[objectName="popup"] {
    box-shadow: 0 4px 12px var(--shadow);
}

/* elevation-3 */
QDialog {
    box-shadow: 0 8px 24px var(--shadow);
}
```

---

## 十二、图标体系

### 图标库选型

| 属性 | 值 |
|------|-----|
| 库名 | Lucide Icons |
| 许可 | MIT License |
| 风格 | 线条风格(Line icons), 1.5px描边 |
| 地址 | https://lucide.dev/ |
| 格式 | SVG (矢量, 支持任意缩放和着色) |

### 资源路径

- 图标文件存放: `resources/icons/lucide/<name>.svg`
- 示例: `resources/icons/lucide/cable.svg`, `resources/icons/lucide/bluetooth.svg`

### 尺寸规范

| 用途 | 尺寸(px) |
|------|----------|
| 导航树节点图标 | 16 |
| 面板标题图标 | 16 |
| 工具栏按钮图标 | 20 |
| 空状态图标 | 48 |
| 命令面板列表项 | 16 |

### 着色规则

- 所有 SVG 使用 `currentColor` 作为填充色
- IconManager 统一管理, 运行时替换为 `ThemeManager::color(TextSecondary)`
- 选中/激活态: 使用 Accent 色
- 禁用态: 使用 TextMuted 色, opacity 40%
- 主题切换时清空缓存重新着色

### 命名规则

- 统一使用 kebab-case 命名
- 示例: `bluetooth-searching`, `bar-chart-2`, `cable`, `terminal`

---

## 十三、空状态设计

### EmptyStateWidget 结构

垂直居中排列, 最大宽度 320px:

```
        ┌────────────────────┐
        │    [icon 48px]     │
        │                    │
        │  标题 (bold 14px)  │
        │                    │
        │ 说明 (muted 12px,  │
        │    word-wrap)      │
        │                    │
        │  [操作按钮(可选)]   │
        └────────────────────┘
```

### 触发场景

| 场景 | icon | title | description | button |
|------|------|-------|-------------|--------|
| 无连接 | plug | "无活跃连接" | "点击左侧导航树创建连接" | "创建连接"(可选) |
| 无数据 | inbox | "暂无数据" | "连接建立后数据将在此显示" | 无 |
| 搜索无结果 | search-x | "未找到匹配项" | "尝试调整搜索关键词" | "清除搜索" |
| 面板加载中 | loader | "加载中" | "正在准备面板内容..." | 无 |

### 规则

- 每个空状态**必须有** icon + title
- description 和 action button 可选
- icon 颜色: TextMuted, 透明度 60%
- 标题颜色: TextSecondary
- 说明文字: TextMuted, word-wrap 模式

---

## 十四、加载状态设计

### LoadingSpinner

| 属性 | 值 |
|------|-----|
| 实现 | QPainter 圆弧旋转 |
| 动画时长 | 1000ms |
| 缓动 | Linear 循环 |
| 颜色 | Accent 色 |
| 线宽 | 3px |
| 尺寸 | 24px (默认), 可配置 |

```cpp
// 使用示例
auto* spinner = new LoadingSpinner(this);
spinner->setFixedSize(24, 24);
spinner->start();
// ... 加载完成后
spinner->stop();
```

### SkeletonWidget

| 属性 | 值 |
|------|-----|
| 实现 | 灰色矩形块 + 渐变微光扫过动画 |
| 动画时长 | 1500ms |
| 缓动 | Linear 循环 |
| 基础色 | bg-tertiary |
| 微光色 | bg-hover 渐变 |

### 使用场景

| 场景 | 组件 | 说明 |
|------|------|------|
| 面板数据加载中 | SkeletonWidget | 显示数据卡片骨架屏 |
| 设备扫描中 | LoadingSpinner | 扫描按钮旁旋转指示 |
| 文件解析中 | LoadingSpinner + 文字 | 进度条区域显示旋转+百分比 |

---

## 十五、键盘快捷键体系

### 全局快捷键

| 快捷键 | 功能 | 上下文 |
|--------|------|--------|
| `Ctrl+F` | 打开搜索栏 | 全局 |
| `Ctrl+P` | 打开命令面板 | 全局 |
| `Ctrl+Shift+R` | 录制开始/停止 | 全局 |
| `Ctrl+Enter` | 发送数据 | 发送区域聚焦时 |
| `Ctrl+L` | 清除终端内容 | 终端聚焦时 |
| `Escape` | 关闭弹出面板/搜索栏/对话框 | 有弹出层时 |

### 快捷键实现规则

- 使用 `QShortcut` 注册, 不重写 `keyPressEvent`
- 快捷键冲突时, 优先级: 弹出层 > 面板 > 全局
- 禁止使用单字母快捷键(避免与输入冲突)
- 快捷键必须在 SettingsManager 中可配置
- 菜单项和工具栏 Tooltip 中显示快捷键提示

---

## 十六、弹窗/对话框规范

### 类型定义

#### 确认弹窗(ConfirmDialog)

```
┌──────────────────────────────────┐
│  标题文字                    [×] │
│  描述说明文字, 说明操作结果      │
│                                  │
│           [取消]  [确认]         │
└──────────────────────────────────┘
```
- 主按钮(确认): 实心 accent + 白字
- 幽灵按钮(取消): 无边框 + TextSecondary

#### 警告弹窗(WarningDialog)

- Warning 色左边框 (3px solid warning)
- 图标: alert-triangle, Warning 色
- 操作: 确认 + 取消

#### 错误弹窗(ErrorDialog)

- Error 色左边框 (3px solid error)
- 图标: alert-circle, Error 色
- 操作: 仅确认按钮

### 统一样式

| 属性 | 值 |
|------|-----|
| 圆角 | 8px |
| 背景 | 毛玻璃效果 (elevation-3) |
| 标题字号 | bold 14px |
| 描述字号 | 13px, TextSecondary |
| 按钮高度 | 32px |
| 按钮最小宽度 | 80px |
| 内边距 | 24px |
| 按钮间距 | 8px |

### 规则

- 所有弹窗必须支持 Escape 键关闭
- 禁止使用 `QMessageBox`, 统一使用自定义弹窗组件
- 弹窗出现时有 200ms OutCubic fade+scale 动画(从95%缩放到100%)
- 确认按钮默认获得焦点

---

## 十七、响应式布局规则

### 断点定义

| 窗口宽度 | 导航树行为 | 面板行为 | 说明 |
|----------|-----------|---------|------|
| ≥1200px | 正常显示(可调宽度) | 正常显示 | 桌面端完整体验 |
| 900~1200px | 可折叠(默认展开) | 正常显示 | 小桌面/大平板 |
| <900px | 自动折叠为图标栏(48px宽) | 面板标题栏隐藏, 全屏内容 | 平板/小窗口 |

### 导航树折叠行为

- **图标栏模式** (<900px): 仅显示连接类型图标(16px), 悬浮展开完整标签
- **可折叠模式** (900~1200px): 默认展开, 用户可手动折叠, 折叠后显示图标栏
- **正常模式** (≥1200px): 始终显示完整导航树

### 面板适配行为

- `<900px`: BasePanel 标题栏自动隐藏, 内容区占满全宽
- 面板最小宽度: 400px (低于此值时触发滚动条)
- 导航树宽度范围: 180px(最小) ~ 280px(默认) ~ 400px(最大)

### 实现要求

- 使用 `QResizeEvent` 监听主窗口尺寸变化
- 断点切换时有 250ms 过渡动画
- 窗口尺寸变化时保存/恢复用户偏好

---

## 十八、主题QSS文件

三个主题文件:
- `resources/themes/dark_terminal.qss`
- `resources/themes/modern_dark.qss`
- `resources/themes/light.qss`

每个必须定义完整语义色板变量。新增控件必须三文件同步。
