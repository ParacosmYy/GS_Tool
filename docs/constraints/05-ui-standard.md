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

## 十、主题QSS文件

三个主题文件:
- `resources/themes/dark_terminal.qss`
- `resources/themes/modern_dark.qss`
- `resources/themes/light.qss`

每个必须定义完整语义色板变量。新增控件必须三文件同步。
