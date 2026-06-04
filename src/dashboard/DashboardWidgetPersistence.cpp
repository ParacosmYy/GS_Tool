/**
 * @file DashboardWidgetPersistence.cpp
 * @brief 仪表盘持久化方法实现 — 文件和配置文件加载/保存
 *
 * 从 DashboardWidget.cpp 拆分而来，包含saveToFile/loadFromFile、
 * saveToProfile/loadFromProfile等持久化方法。
 */

#include "dashboard/DashboardWidget.h"
#include "dashboard/DashboardSerializer.h"

/** @brief 获取序列化器实例 @return 序列化器指针 */
DashboardSerializer* DashboardWidget::serializer() const
{
    return m_serializer;
}

/** @brief 保存当前布局到JSON文件 @param filePath 目标文件路径 @param name 布局名称 @return true=保存成功 */
bool DashboardWidget::saveToFile(const QString &filePath, const QString &name)
{
    const QList<DashboardItemConfig> items = saveToItems();
    const bool ok = m_serializer->saveToFile(filePath, name, kColumns, items);
    if (ok) {
        ++m_stats.totalFullSaves;
        emit savedToFile(filePath);
    }
    return ok;
}

/** @brief 从JSON文件加载布局 @param filePath 源文件路径 @return true=加载成功 */
bool DashboardWidget::loadFromFile(const QString &filePath)
{
    QString name;
    int columns = kColumns;
    QList<DashboardItemConfig> items;

    const bool ok = m_serializer->loadFromFile(filePath, name, columns, items);
    if (!ok) {
        return false;
    }

    loadFromItems(items, columns);
    emit loadedFromFile(filePath);
    return true;
}

/** @brief 保存当前布局到QSettings命名配置文件 @param profileName 配置文件名称 @return true=保存成功 */
bool DashboardWidget::saveToProfile(const QString &profileName)
{
    const QList<DashboardItemConfig> items = saveToItems();
    const QString name = tr("布局-%1").arg(profileName);
    const bool ok = m_serializer->saveToProfile(profileName, name, kColumns, items);
    if (ok) {
        ++m_stats.totalFullSaves;
        emit savedToProfile(profileName);
    }
    return ok;
}

/** @brief 从QSettings命名配置文件加载布局 @param profileName 配置文件名称 @return true=加载成功 */
bool DashboardWidget::loadFromProfile(const QString &profileName)
{
    QString name;
    int columns = kColumns;
    QList<DashboardItemConfig> items;

    const bool ok = m_serializer->loadFromProfile(profileName, name, columns, items);
    if (!ok) {
        return false;
    }

    loadFromItems(items, columns);
    emit loadedFromProfile(profileName);
    return true;
}

/** @brief 获取指定索引的子组件 @param index 索引 @return 子控件指针，越界返回nullptr */
QWidget *DashboardWidget::componentAt(int index) const
{
    if (index < 0 || index >= m_components.size()) {
        return nullptr;
    }
    return m_components.at(index);
}

/** @brief 获取子组件总数 @return 数量 */
int DashboardWidget::componentCount() const
{
    return m_components.size();
}

/** @brief 获取网格列数 @return 列数 */
int DashboardWidget::gridColumns() const
{
    return kColumns;
}
