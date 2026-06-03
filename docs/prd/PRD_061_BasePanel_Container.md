# PRD-061: BasePanel容器组件 — 统一面板标题栏/折叠/动画

## 背景
当前项目中各面板(SerialConfigPanel, DataStatistics, TrafficMonitorWidget等)各自实现标题栏和折叠逻辑，导致样式不统一、动画行为不一致、QSS维护成本高。需要引入BasePanel容器组件，以Wrapper模式统一包装所有面板，提供一致的标题栏(图标+标题+折叠按钮)、展开/折叠动画和阴影渲染。

排除范围: TerminalWidget、TerminalSearchBar、QuickCommandBar不参与面板堆叠，不使用BasePanel包装。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | BasePanel容器: 标题栏(icon + title + collapse button) + 内容区 + 阴影 | P0 | core/widgets/ |
| R2 | 展开动画: 250ms fade+slide InOutCubic | P0 | core/widgets/ |
| R3 | 折叠动画: 200ms fade+slide OutInCubic | P0 | core/widgets/ |
| R4 | PanelManager集成: m_wrappers map以Wrapper模式管理面板 | P0 | core/PanelManager |
| R5 | 折叠状态持久化: SettingsManager存储各面板collapsed状态 | P1 | core/SettingsManager |
| R6 | QSS规则: 3套主题的BasePanel样式定义 | P0 | resources/themes/ |

## 接口设计

### BasePanel类
```cpp
/**
 * @brief 面板容器组件 -- 统一标题栏/折叠/展开动画
 *
 * 以Wrapper模式包装任意QWidget，提供:
 *   - 统一标题栏(IconManager图标 + 标题文本 + 折叠按钮)
 *   - 展开/折叠动画(QPropertyAnimation)
 *   - 阴影渲染(QPainter paintEvent)
 */
class BasePanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit BasePanel(const QString& title, const QString& iconName,
                       QWidget* content, QWidget* parent = nullptr);

    /** @brief 设置内容widget(BasePanel接管ownership) */
    void setContent(QWidget* content);
    /** @brief 获取内容widget */
    QWidget* content() const;
    /** @brief 是否已折叠 */
    bool isCollapsed() const;
    /** @brief 设置折叠状态(带动画) */
    void setCollapsed(bool collapsed);

signals:
    /** @brief 折叠状态变更 */
    void collapsedChanged(bool collapsed);

private slots:
    void onThemeChanged();

private:
    void setupUi();
    void paintEvent(QPaintEvent* event) override;

    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    AnimatedButton* m_collapseBtn;
    QWidget* m_contentArea;
    QWidget* m_content = nullptr;
    QPropertyAnimation* m_anim = nullptr;
    qreal m_animProgress = 1.0;
    bool m_collapsed = false;
};
```

### PanelManager集成
```cpp
// PanelManager.h 新增成员
QMap<QString, BasePanel*> m_wrappers;  ///< panelId → BasePanel包装器

/** @brief 注册面板到容器(自动创建BasePanel包装) */
void registerPanel(const QString& id, const QString& title,
                   const QString& icon, QWidget* content);
```

## 依赖的公共组件
- ThemeManager (core/theme/ThemeManager.h) — color()获取主题色, themeChanged信号
- AnimatedButton (core/widgets/AnimatedButton.h) — 折叠按钮
- IconManager (待PRD-062实现) — 图标加载
- PanelManager (core/PanelManager.h) — 面板注册管理
- SettingsManager (utils/SettingsManager.h) — 折叠状态持久化

## 设计模式
- **Wrapper/Decorator模式**: BasePanel包装QWidget，不修改原面板代码
- **观察者模式**: 监听ThemeManager::themeChanged重绘阴影和更新QSS
- **状态模式**: 折叠/展开两个状态通过QPropertyAnimation过渡

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/core/widgets/BasePanel.h | 新增 | 无 |
| src/core/widgets/BasePanel.cpp | 新增 | 无 |
| src/core/PanelManager.h | 修改(+m_wrappers) | 低 |
| src/core/PanelManager.cpp | 修改(registerPanel) | 低 |
| resources/themes/*.qss | 修改(BasePanel样式) | 低 |
| SerialConfigPanel.cpp | 修改(接入BasePanel) | 中 |

## 验收标准
1. 所有面板标题栏样式统一: 左侧图标 + 标题文本 + 右侧折叠按钮
2. 点击折叠按钮, 内容区200ms滑动消失; 再次点击250ms滑动展开
3. 面板周围绘制柔和阴影(BorderSecondary, 10px模糊半径)
4. 3套主题(Light/Dark/OneDark)切换后BasePanel样式正确
5. 折叠状态在应用重启后保持(通过SettingsManager持久化)
6. TerminalWidget/SearchBar/QuickCmdBar不被BasePanel包装
7. 所有文件.h ≤ 200行, .cpp ≤ 500行
8. 编译零错误, EmbedDebug.bat启动正常
