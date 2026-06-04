/**
 * @file DashboardModelItems.cpp
 * @brief 仪表盘模型 —— 数据通道查询、组件统计、布局生命周期统计实现
 *
 * 从 DashboardModel.cpp 拆分而来，集中管理通道值查询、通道统计、
 * 组件类型统计、布局变更跟踪、配置文件管理统计、布局生命周期统计
 * 以及全局统计重置等方法。
 */

#include "dashboard/DashboardModel.h"

// ==================== 数据通道查询 ====================

/** @brief 获取所有通道名称 @return 通道名称列表(按插入顺序) */
QStringList DashboardModel::channelNames() const
{
    return m_channels.keys();
}

/** @brief 获取指定通道的当前值 @param name 通道名称 @return 通道值，通道不存在时返回0.0 */
double DashboardModel::value(const QString &name) const
{
    return m_channels.value(name, 0.0);
}

/** @brief 获取指定通道的最小值 @param name 通道名称 @return 最小值，通道不存在时返回0.0 */
double DashboardModel::channelMin(const QString &name) const
{
    return m_channelMin.value(name, 0.0);
}

/** @brief 获取指定通道的最大值 @param name 通道名称 @return 最大值，通道不存在时返回0.0 */
double DashboardModel::channelMax(const QString &name) const
{
    return m_channelMax.value(name, 0.0);
}

/** @brief 获取指定通道的值变更次数 @param name 通道名称 @return 变更次数 */
quint64 DashboardModel::channelChangeCount(const QString &name) const
{
    return m_channelChangeCount.value(name, 0);
}

/** @brief 获取总更新次数（所有通道累计） @return 累计更新次数 */
quint64 DashboardModel::totalUpdateCount() const
{
    return m_totalUpdateCount;
}

/** @brief 重置所有通道统计（min/max/changeCount/totalUpdateCount） */
void DashboardModel::resetChannelStatistics()
{
    m_channelMin.clear();
    m_channelMax.clear();
    m_channelChangeCount.clear();
    m_totalUpdateCount = 0;
}

// ==================== 组件类型统计 ====================

/** @brief 获取累计创建的组件总数(含已删除) @return 创建总数 */
quint64 DashboardModel::totalWidgetsCreated() const { return m_totalWidgetsCreated; }

/** @brief 获取累计删除的组件总数 @return 删除总数 */
quint64 DashboardModel::totalWidgetsRemoved() const { return m_totalWidgetsRemoved; }

/** @brief 获取当前活跃组件数量 @return 当前配置数 */
quint64 DashboardModel::activeWidgetCount() const { return static_cast<quint64>(m_configs.size()); }

/** @brief 获取指定类型的累计创建计数 @param type 组件类型 @return 该类型创建总数 */
quint64 DashboardModel::widgetsCreatedByType(const QString &type) const
{
    return m_widgetsCreatedByType.value(type, 0);
}

// ==================== 布局变更跟踪 ====================

/** @brief 获取累计布局变更次数 @return 变更总数 */
quint64 DashboardModel::layoutChanges() const { return m_layoutChanges; }

// ==================== 配置文件管理统计 ====================

/** @brief 获取累计配置保存次数 @return 保存总数 */
quint64 DashboardModel::profileSaves() const { return m_profileSaves; }

/** @brief 获取累计配置加载次数 @return 加载总数 */
quint64 DashboardModel::profileLoads() const { return m_profileLoads; }

/** @brief 获取累计配置删除次数 @return 删除总数 */
quint64 DashboardModel::profileDeletes() const { return m_profileDeletes; }

// ==================== 布局生命周期统计 ====================

/** @brief 获取累计布局变更次数(增/删组件触发) @return 变更总数 */
quint64 DashboardModel::totalLayoutChanges() const { return m_totalLayoutChanges; }

/** @brief 获取累计组件添加次数 @return 添加总数 */
quint64 DashboardModel::totalWidgetAdditions() const { return m_totalWidgetAdditions; }

/** @brief 获取累计组件移除次数 @return 移除总数 */
quint64 DashboardModel::totalWidgetRemovals() const { return m_totalWidgetRemovals; }

/** @brief 获取累计布局保存次数 @return 保存总数 */
quint64 DashboardModel::totalLayoutSaves() const { return m_totalLayoutSaves; }

/** @brief 获取累计布局加载次数 @return 加载总数 */
quint64 DashboardModel::totalLayoutLoads() const { return m_totalLayoutLoads; }

/** @brief 重置所有统计计数器(通道统计+组件统计+布局统计+配置文件统计+布局生命周期统计) */
void DashboardModel::resetAllStatistics()
{
    /* 通道统计 */
    m_channelMin.clear();
    m_channelMax.clear();
    m_channelChangeCount.clear();
    m_totalUpdateCount = 0;
    /* 组件统计 */
    m_totalWidgetsCreated = 0;
    m_totalWidgetsRemoved = 0;
    m_widgetsCreatedByType.clear();
    m_layoutChanges = 0;
    /* 配置文件统计 */
    m_profileSaves = 0;
    m_profileLoads = 0;
    m_profileDeletes = 0;
    /* 布局生命周期统计 */
    m_totalLayoutChanges = 0;
    m_totalWidgetAdditions = 0;
    m_totalWidgetRemovals = 0;
    m_totalLayoutSaves = 0;
    m_totalLayoutLoads = 0;
}
