/**
 * @file PluginConfigPanelUI.cpp
 * @brief 插件配置面板UI交互实现 — 信号连接、列表刷新和详情面板更新
 *
 * 从 PluginConfigPanel.cpp 拆分而来，包含setupUI信号连接、
 * refreshList列表刷新和updateDetailPanel详情面板更新。
 */

#include "plugin/PluginConfigPanel.h"
#include "plugin/PluginManager.h"

#include <QListWidget>
#include <QFileDialog>
#include <QApplication>
#include <QFileInfo>

/**
 * @brief 从 PluginManager 刷新插件列表显示
 *
 * 清空列表后，遍历已加载插件名称重新填充。
 */
void PluginConfigPanel::refreshList()
{
    if (!m_manager) {
        return;
    }
    ++m_stats.totalDisplays;

    m_pluginList->clear();
    const QStringList names = m_manager->loadedPluginNames();
    for (const QString& name : names) {
        auto* item = new QListWidgetItem(
            name + tr(" (loaded)"), m_pluginList);
        item->setData(Qt::UserRole, name);
    }

    /* 重置详情面板 */
    m_detailNameLabel->setText(tr("名称：-"));
    m_detailVersionLabel->setText(tr("版本：-"));
    m_detailStatusLabel->setText(tr("状态：-"));
    m_detailDescEdit->clear();
}

/**
 * @brief 更新详情面板显示指定插件的元数据
 *
 * 从 PluginManager 获取插件名称、版本、描述并刷新右侧详情区域。
 *
 * @param pluginName 已加载插件的名称
 */
void PluginConfigPanel::updateDetailPanel(const QString& pluginName)
{
    if (!m_manager) {
        return;
    }

    m_detailNameLabel->setText(tr("名称：%1").arg(pluginName));
    m_detailVersionLabel->setText(
        tr("版本：%1").arg(m_manager->pluginVersion(pluginName)));
    m_detailStatusLabel->setText(tr("状态：已加载"));
    m_detailDescEdit->setPlainText(
        m_manager->pluginDescription(pluginName));
}
