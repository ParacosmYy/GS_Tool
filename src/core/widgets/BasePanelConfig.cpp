/**
 * @file BasePanelConfig.cpp
 * @brief 基础面板 - 配置与属性设置方法实现
 *
 * 从 BasePanel.cpp 拆分而来，包含标题、图标、折叠、透明度等
 * 配置属性的设置与查询方法。
 */

#include "core/widgets/BasePanel.h"

#include <QLabel>
#include <QPushButton>
#include <QGraphicsOpacityEffect>

/** @brief 设置标题栏文字 @param title 新的标题文字 */
void BasePanel::setTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
    ++m_totalTitleChanges;
}

/** @brief 获取当前标题栏文字 @return 标题文字，标签未创建时返回空字符串 */
QString BasePanel::title() const
{
    return m_titleLabel ? m_titleLabel->text() : QString();
}

/** @brief 设置标题栏图标名称，显示首字符作为占位 @param name 图标名称 */
void BasePanel::setIconName(const QString& name)
{
    m_iconName = name;
    if (m_iconLabel) {
        m_iconLabel->setText(name.isEmpty() ? QString() : QString(name.at(0)));
    }
}

/** @brief 设置面板是否可折叠 @param enabled true允许折叠，false禁止折叠 */
void BasePanel::setCollapsible(bool enabled)
{
    m_collapsible = enabled;
    if (m_collapseBtn) {
        m_collapseBtn->setVisible(enabled);
    }
    if (!enabled && m_collapsed) {
        setCollapsed(false);
    }
}

/** @brief 查询面板是否可折叠 @return true表示允许折叠 */
bool BasePanel::isCollapsible() const
{
    return m_collapsible;
}

/** @brief 设置面板折叠状态 @param collapsed true折叠内容区域，false展开内容区域 */
void BasePanel::setCollapsed(bool collapsed)
{
    if (collapsed == m_collapsed) return;
    m_collapsed = collapsed;

    if (collapsed) ++m_totalCollapses; else ++m_totalExpansions;

    if (m_contentArea) {
        m_contentArea->setVisible(!collapsed);
    }
    updateCollapseIcon();
    emit collapsedChanged(collapsed);
}

/** @brief 查询面板当前是否处于折叠状态 @return true表示已折叠 */
bool BasePanel::isCollapsed() const
{
    return m_collapsed;
}

/** @brief 获取面板透明度 @return 透明度值(0.0~1.0) */
qreal BasePanel::panelOpacity() const
{
    return m_panelOpacity;
}

/** @brief 设置面板透明度 @param opacity 透明度值(0.0完全透明~1.0完全不透明) */
void BasePanel::setPanelOpacity(qreal opacity)
{
    m_panelOpacity = opacity;
    if (m_opacityEffect) {
        m_opacityEffect->setOpacity(opacity);
    }
}
