/**
 * @file NavigationControllerQuery.cpp
 * @brief 导航控制器 - 面板查询与统计接口实现
 *
 * 从 NavigationController.cpp 拆分而来，包含面板查找、
 * 索引恢复、统计计数器访问/重置和主题刷新回调方法。
 */

#include "core/navigation/NavigationController.h"
#include "core/theme/ThemeManager.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QPainter>
#include <QPixmap>
#include <QCoreApplication>

/** @brief 创建导航树连接类型指示圆点图标(8x8透明底+抗锯齿彩色圆点) @param color 圆点颜色 @return 图标实例 */
static QIcon createDotIcon(const QColor& color)
{
    QPixmap dot(8, 8);
    dot.fill(Qt::transparent);
    QPainter painter(&dot);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(1, 1, 6, 6);
    return QIcon(dot);
}

/** @brief 通过翻译后的名称查找对应的panel widget @param translatedName 翻译后的面板名称 @return 对应的widget指针，未找到返回nullptr */
QWidget* NavigationController::lookupPanel(const QString& translatedName) const
{
    for (const auto& mapping : m_navPanelMappings) {
        if (translatedName == QCoreApplication::translate("MainWindow", mapping.name)) {
            return mapping.widget;
        }
    }
    return nullptr;
}

/** @brief 获取指定导航分类下的首个面板 @param categoryKey 原始分类翻译键或翻译后的分类名 @return 首个面板指针，未找到返回nullptr */
QWidget* NavigationController::firstPanelInCategory(const QString& categoryKey) const
{
    for (const auto& mapping : m_navPanelMappings) {
        const QString rawCategory = QString::fromUtf8(mapping.category);
        const QString translatedCategory = QCoreApplication::translate("Nav", mapping.category);
        if ((categoryKey == rawCategory || categoryKey == translatedCategory) && mapping.widget) {
            return mapping.widget;
        }
    }
    return nullptr;
}

/** @brief 获取指定分类内相对当前面板的下一个面板，当前不在该分类时返回首个面板 */
QWidget* NavigationController::nextPanelInCategory(const QString& categoryKey) const
{
    QVector<QWidget*> categoryPanels;
    for (const auto& mapping : m_navPanelMappings) {
        const QString rawCategory = QString::fromUtf8(mapping.category);
        const QString translatedCategory = QCoreApplication::translate("Nav", mapping.category);
        if ((categoryKey == rawCategory || categoryKey == translatedCategory) && mapping.widget) {
            categoryPanels.append(mapping.widget);
        }
    }

    if (categoryPanels.isEmpty()) {
        return nullptr;
    }

    const int currentIndex = categoryPanels.indexOf(m_currentPanel);
    if (currentIndex < 0) {
        return categoryPanels.first();
    }
    return categoryPanels.at((currentIndex + 1) % categoryPanels.size());
}

/** @brief 获取面板所属导航分类原始键 @param panel 面板指针 @return 分类翻译键，未找到返回空字符串 */
QString NavigationController::categoryForPanel(QWidget* panel) const
{
    if (!panel) {
        return {};
    }

    for (const auto& mapping : m_navPanelMappings) {
        if (mapping.widget == panel) {
            return QString::fromUtf8(mapping.category);
        }
    }
    return {};
}

// ---- 统计计数器实现 ----

/** @brief 获取导航切换总次数 @return 切换总次数 */
quint64 NavigationController::totalNavigations() const
{
    return m_totalNavigations;
}

/** @brief 获取面板实际变更总次数 @return 面板变更次数 */
quint64 NavigationController::totalPanelSwitches() const
{
    return m_totalPanelSwitches;
}

/** @brief 获取导航树展开/折叠操作总次数 @return 展开/折叠操作次数 */
quint64 NavigationController::totalTreeExpansions() const
{
    return m_totalTreeExpansions;
}

/** @brief 获取导航搜索总次数 @return 搜索次数 */
quint64 NavigationController::totalSearches() const
{
    return m_totalSearches;
}

/** @brief 获取呼吸动画启动总次数 @return 启动次数 */
quint64 NavigationController::totalBreathingStarts() const
{
    return m_totalBreathingStarts;
}

/** @brief 获取呼吸动画停止总次数 @return 停止次数 */
quint64 NavigationController::totalBreathingStops() const
{
    return m_totalBreathingStops;
}

/** @brief 获取分类节点点击总次数 @return 分类点击次数 */
quint64 NavigationController::totalCategoryClicks() const
{
    return m_totalCategoryClicks;
}

/** @brief 获取通过索引恢复面板总次数(会话恢复) @return 恢复次数 */
quint64 NavigationController::totalRestoresByIndex() const
{
    return m_totalRestoresByIndex;
}

/** @brief 获取导航树重建总次数(buildNavTree调用) @return 重建次数 */
quint64 NavigationController::totalNavTreeRebuilds() const
{
    return m_totalNavTreeRebuilds;
}

/** @brief 重置所有导航统计计数器(切换/面板变更/展开/搜索/分类点击/恢复/重建)为零 */
void NavigationController::resetNavigationStatistics()
{
    m_totalNavigations = 0;
    m_totalPanelSwitches = 0;
    m_totalTreeExpansions = 0;
    m_totalSearches = 0;
    m_totalBreathingStarts = 0;
    m_totalBreathingStops = 0;
    m_totalCategoryClicks = 0;
    m_totalRestoresByIndex = 0;
    m_totalNavTreeRebuilds = 0;
}

/** @brief 主题切换时刷新导航树圆点图标颜色，遍历树模型根节点的所有category分组统一刷新为Accent色 */
void NavigationController::onThemeChanged()
{
    if (!m_navTree) return;

    auto* model = qobject_cast<QStandardItemModel*>(m_navTree->model());
    if (!model) return;

    auto* root = model->invisibleRootItem();
    if (!root) return;

    QColor dotColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
    for (int i = 0; i < root->rowCount(); ++i) {
        auto* catItem = root->child(i);
        if (catItem) {
            catItem->setIcon(createDotIcon(dotColor));
        }
    }
}
