/**
 * @file IconNavBar.h
 * @brief 图标导航栏 — 垂直图标栏用于三栏布局模式
 *
 * 56px宽的垂直图标导航栏，替代完整QTreeView用于紧凑布局。
 * 点击图标展开对应分类的侧边栏面板列表。
 * 默认关闭，通过 SettingsManager "ui/iconNavBar" 配置开启。
 *
 * 设计模式: 策略(Strategy) — 替代QTreeView的另一种导航方式
 * 协作: NavigationController / PanelManager / SettingsManager
 */
#ifndef ICON_NAV_BAR_H
#define ICON_NAV_BAR_H

#include <QWidget>
#include <QVector>
#include <QMap>

class QPushButton;
class QVBoxLayout;
class QButtonGroup;

/**
 * @brief 导航分类条目
 */
struct NavCategory {
    QString id;           ///< 分类ID (如 "connection", "terminal")
    QString iconName;     ///< Lucide图标名 (如 "cable", "terminal")
    QString label;        ///< 显示标签 (使用tr())
    QStringList panelIds; ///< 该分类下的面板ID列表
};

/**
 * @brief 图标导航栏 — 三栏布局的紧凑导航
 */
class IconNavBar : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造图标导航栏
     * @param parent 父widget
     */
    explicit IconNavBar(QWidget* parent = nullptr);

    /** @brief 设置导航分类列表 */
    void setCategories(const QVector<NavCategory>& categories);

    /** @brief 设置当前激活分类ID */
    void setActiveCategory(const QString& id);

    /** @brief 获取固定宽度(56px) */
    int navBarWidth() const { return 56; }

    // ── 统计计数器 ──

    /** @brief 获取分类切换总次数 */
    quint64 totalCategorySwitches() const { return m_totalCategorySwitches; }
    /** @brief 获取按钮点击总次数 */
    quint64 totalButtonClicks() const { return m_totalButtonClicks; }
    /** @brief 重置所有统计计数器 */
    void resetNavStatistics();

signals:
    /** @brief 分类被点击 */
    void categoryClicked(const QString& id);

protected:
    /** @brief 绘制左侧激活指示线 */
    void paintEvent(QPaintEvent* event) override;

private:
    /** @brief 创建分类按钮 */
    QPushButton* createCategoryButton(const NavCategory& category);

    QVBoxLayout* m_layout = nullptr;         ///< objectName="iconNavBarLayout"
    QButtonGroup* m_buttonGroup = nullptr;   ///< 按钮分组(互斥选择)
    QMap<QString, QPushButton*> m_buttons;   ///< 分类ID→按钮映射
    QString m_activeCategory;                 ///< 当前激活分类ID
    int m_activeIndex = -1;                   ///< 当前激活索引(用于指示线)

    // ── 统计计数器 ──
    quint64 m_totalCategorySwitches = 0;      ///< 分类切换次数
    quint64 m_totalButtonClicks = 0;          ///< 按钮点击次数
};

#endif // ICON_NAV_BAR_H
