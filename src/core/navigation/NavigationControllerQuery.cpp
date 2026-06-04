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
