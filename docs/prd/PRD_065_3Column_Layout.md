# PRD-065: 三栏布局重构 — IconNavBar + Sidebar + Content

## 背景
当前MainWindow采用QTreeView侧边栏 + 内容区的两栏布局。随着面板增至44个，QTreeView层级深，鼠标操作效率低。引入IconNavBar垂直图标栏，将导航精简为7个分类图标，点击后Sidebar展开对应分类面板列表。IconNavBar与QTreeView双向同步，功能通过SettingsManager开关控制(feature flag默认关闭)，不影响现有布局。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | IconNavBar: 56px宽垂直图标条, 7个分类图标(40×40px) | P0 | core/widgets/ |
| R2 | 活跃项样式: BgHover背景 + Accent左侧边框 | P0 | core/widgets/ |
| R3 | 与QTreeView双向同步: IconNavBar点击→TreeView展开, TreeView点击→NavBar高亮 | P0 | core/MainWindow |
| R4 | Feature flag: SettingsManager "ui/iconNavBar" 默认false | P0 | utils/SettingsManager |
| R5 | 7大分类: 连接/终端/数据/协议/调试/工具/系统 | P0 | core/widgets/ |
| R6 | MainWindow::createNavigationArea()重构 | P0 | core/MainWindow |

## 接口设计

### IconNavBar类
```cpp
/**
 * @brief 垂直图标导航栏 -- 7大分类快速切换
 *
 * 布局: 56px宽垂直条，顶部7个分类图标(40×40px)，底部留空。
 * 每个分类图标对应QTreeView的一级节点。
 * 点击图标 → 发射categorySelected → QTreeView展开对应分类。
 * QTreeView点击 → NavigationController通知 → NavBar更新高亮。
 */
class IconNavBar : public QWidget {
    Q_OBJECT

public:
    /** @brief 面板分类枚举 */
    enum class Category {
        Connection,  ///< 连接(串口/网络/USB)
        Terminal,    ///< 终端(主终端/搜索/过滤)
        Data,        ///< 数据(统计/流量/书签)
        Protocol,    ///< 协议(Modbus/帧解析/协议桥)
        Debug,       ///< 调试(图表/仪表盘/RTT)
        Tools,       ///< 工具(OTA/转换/校验/时间戳)
        System       ///< 系统(设置/插件/性能)
    };
    Q_ENUM(Category)

    explicit IconNavBar(QWidget* parent = nullptr);

    /** @brief 设置当前活跃分类(由外部导航同步) */
    void setActiveCategory(Category cat);

signals:
    /** @brief 用户点击分类图标 */
    void categorySelected(Category cat);

private slots:
    void onThemeChanged();

private:
    void setupUi();
    void paintEvent(QPaintEvent* event) override;

    struct NavItem {
        Category category;
        QString iconName;
        QString tooltip;
        QPushButton* button = nullptr;
    };
    QList<NavItem> m_items;
    Category m_active = Category::Connection;
};
```

### MainWindow集成
```cpp
// MainWindow::createNavigationArea()重构
QWidget* MainWindow::createNavigationArea()
{
    auto* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (SettingsManager::instance().value("ui/iconNavBar", false).toBool()) {
        m_iconNavBar = new IconNavBar(this);
        layout->addWidget(m_iconNavBar);
        // 双向同步连接
        connect(m_iconNavBar, &IconNavBar::categorySelected,
                m_navController, &NavigationController::expandCategory);
        connect(m_navController, &NavigationController::panelSwitched,
                m_iconNavBar, &IconNavBar::setActiveCategory);
    }

    m_treeView = new QTreeView(this);
    layout->addWidget(m_treeView, 1);
    return container;
}
```

## 依赖的公共组件
- ThemeManager (core/theme/ThemeManager.h) — 颜色, themeChanged
- IconManager (core/theme/IconManager.h) — 分类图标
- NavigationController (core/NavigationController.h) — 导航同步
- PanelManager (core/PanelManager.h) — 面板分类映射
- SettingsManager (utils/SettingsManager.h) — feature flag

## 设计模式
- **观察者模式**: IconNavBar ↔ NavigationController双向信号同步
- **Feature Flag模式**: SettingsManager控制功能开关，运行时可切换
- **枚举映射**: Category枚举 ↔ QTreeView一级节点索引的静态映射表

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/core/widgets/IconNavBar.h | 新增 | 无 |
| src/core/widgets/IconNavBar.cpp | 新增 | 无 |
| src/core/MainWindow.cpp | 修改(createNavigationArea) | 中(布局重构) |
| src/core/NavigationController.h | 修改(expandCategory信号) | 低 |
| resources/themes/*.qss | 修改(IconNavBar样式) | 低 |

## 验收标准
1. SettingsManager "ui/iconNavBar"=false时, 布局与当前完全一致(无回归)
2. flag=true时, 左侧出现56px宽IconNavBar，显示7个分类图标
3. 点击IconNavBar"终端"图标，QTreeView展开终端分类并高亮第一个面板
4. 在QTreeView中点击"数据统计"，IconNavBar自动高亮"数据"分类图标
5. 活跃图标样式: BgHover背景 + 左侧2px Accent边框
6. tooltip显示分类名称(tr()包裹)
7. 主题切换后IconNavBar颜色正确
8. .h ≤ 200行, .cpp ≤ 500行
9. 编译零错误
