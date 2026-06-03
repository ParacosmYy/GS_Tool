# PRD-064: CommandPalette命令面板 — Ctrl+P全局模糊搜索

## 背景
项目面板数量已达44个，加上工具栏操作，用户查找目标面板或操作越来越困难。参考VSCode的Ctrl+P命令面板，实现一个全局模糊搜索浮层，让用户通过键盘快速定位并跳转到任意面板或触发常用操作。不改变现有导航结构，作为加速器补充。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 半透明遮罩 + 居中面板(480×400px)，Esc关闭 | P0 | core/widgets/ |
| R2 | QLineEdit搜索框 + QListView结果列表 | P0 | core/widgets/ |
| R3 | 模糊搜索算法: 前缀+子序列匹配，高亮匹配字符 | P0 | core/widgets/ |
| R4 | 注册44个面板 + 主题切换/录制/导出/设置操作 | P0 | core/MainWindow |
| R5 | 键盘导航: ↑↓选择, Enter确认, Esc关闭 | P0 | core/widgets/ |
| R6 | Ctrl+P全局快捷键绑定 | P0 | core/MainWindow |

## 接口设计

### CommandPalette类
```cpp
/**
 * @brief 命令面板 -- Ctrl+P全局模糊搜索面板和操作
 *
 * 搜索项来源:
 *   - PanelManager注册的所有面板(44个)
 *   - 工具栏操作: 主题切换/录制开关/数据导出/设置
 * 匹配算法: 前缀匹配优先，其次子序列匹配，按权重排序
 */
class CommandPalette : public QWidget {
    Q_OBJECT

public:
    explicit CommandPalette(QWidget* parent = nullptr);

    /** @brief 注册可搜索项 */
    void registerItem(const QString& id, const QString& label,
                      const QString& category, const QString& iconName,
                      std::function<void()> action);

    /** @brief 显示面板(带动画) */
    void showPalette();
    /** @brief 隐藏面板(带动画) */
    void hidePalette();

signals:
    /** @brief 选中某个命令项 */
    void itemSelected(const QString& id);

private slots:
    void onSearchTextChanged(const QString& text);
    void onItemActivated(const QModelIndex& index);

private:
    void setupUi();
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void fuzzyFilter(const QString& query);
    int matchScore(const QString& text, const QString& query) const;

    QLineEdit* m_searchInput;
    QListView* m_resultList;
    QStandardItemModel* m_model;
    QWidget* m_backdrop;

    /** @brief 命令项定义 */
    struct CommandItem {
        QString id;
        QString label;
        QString category;    ///< 分类: 面板/操作
        QString iconName;
        std::function<void()> action;
    };
    QList<CommandItem> m_items;       ///< 全部注册项
    QList<int> m_filteredIndices;      ///< 过滤后的索引
};
```

### MainWindow注册示例
```cpp
// MainWindow.cpp中注册面板项
auto* palette = new CommandPalette(this);
for (const auto& panel : panelManager->allPanels()) {
    palette->registerItem(
        panel.id(), panel.title(), tr("面板"),
        panel.iconName(),
        [this, id = panel.id()](){ navigateToPanel(id); }
    );
}
// 注册操作项
palette->registerItem("theme-switch", tr("切换主题"), tr("操作"),
                       "special/palette", [this](){ cycleTheme(); });
```

## 依赖的公共组件
- ThemeManager (core/theme/ThemeManager.h) — 半透明遮罩颜色, 面板背景色
- IconManager (core/theme/IconManager.h) — 各项图标显示
- PanelManager (core/PanelManager.h) — 面板列表数据源
- NavigationController (core/NavigationController.h) — 面板跳转

## 设计模式
- **命令模式**: CommandItem封装id + label + action，解耦搜索与执行
- **注册模式**: registerItem()动态注册，MainWindow负责填充命令表
- **观察者模式**: 搜索文本变更触发过滤和结果更新

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/core/widgets/CommandPalette.h | 新增 | 无 |
| src/core/widgets/CommandPalette.cpp | 新增 | 无 |
| src/core/MainWindow.cpp | 修改(注册+快捷键) | 低 |
| resources/themes/*.qss | 修改(CommandPalette样式) | 低 |

## 验收标准
1. Ctrl+P弹出命令面板，Esc关闭，动画流畅(200ms)
2. 输入"seri"模糊匹配到"串口配置"面板，匹配字符高亮
3. ↑↓键在结果列表中移动选中项，Enter执行跳转
4. 所有44个面板和工具栏操作均注册为可搜索项
5. 面板显示在屏幕中央(480×400px)，背景半透明遮罩
6. 搜索空字符串时显示全部项，按分类分组
7. .h ≤ 200行, .cpp ≤ 500行
8. 编译零错误
