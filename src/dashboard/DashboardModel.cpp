/**
 * @file DashboardModel.cpp
 * @brief 仪表盘配置模型实现
 *
 * 管理组件配置的增删查、数据通道的实时值更新，
 * 使用 QJsonDocument 实现文件持久化。
 */

#include "dashboard/DashboardModel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

/** @brief 构造函数 @param parent 父对象 */
DashboardModel::DashboardModel(QObject *parent)
    : QObject(parent)
{
}

/** @brief 析构函数 */
DashboardModel::~DashboardModel() = default;

/** @brief 添加组件配置，更新组件类型计数和布局变更计数 @param config 组件配置 */
void DashboardModel::addComponentConfig(const QVariantMap &config)
{
    m_configs.append(config);
    ++m_totalWidgetsCreated;
    ++m_layoutChanges;
    ++m_totalLayoutChanges;
    ++m_totalWidgetAdditions;
    const QString type = config.value(QStringLiteral("type")).toString();
    if (!type.isEmpty()) {
        ++m_widgetsCreatedByType[type];
    }
    emit configChanged();
}

/** @brief 移除指定索引的组件配置，更新删除计数和布局变更计数 @param index 配置索引 */
void DashboardModel::removeComponentConfig(int index)
{
    if (index >= 0 && index < m_configs.size()) {
        m_configs.removeAt(index);
        ++m_totalWidgetsRemoved;
        ++m_layoutChanges;
        ++m_totalLayoutChanges;
        ++m_totalWidgetRemovals;
        emit configChanged();
    }
}

/** @brief 获取所有组件配置 @return 配置列表 */
QList<QVariantMap> DashboardModel::componentConfigs() const
{
    return m_configs;
}

/** @brief 获取配置数量 @return 数量 */
int DashboardModel::configCount() const
{
    return m_configs.size();
}

/** @brief 将配置保存到文件，序列化m_configs为JSON数组并写入文件 @param filePath 目标文件路径 @return true=成功 */
bool DashboardModel::saveToFile(const QString &filePath) const
{
    if (filePath.isEmpty()) {
        return false;
    }

    QJsonArray arr;
    for (const auto &config : m_configs) {
        QJsonObject obj = QJsonObject::fromVariantMap(config);
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    /* 统计：配置文件保存计数（const方法中修改mutable计数器） */
    ++m_profileSaves;
    ++m_totalLayoutSaves;
    return true;
}

/** @brief 从文件加载配置，从JSON文件读取配置并填充到m_configs @param filePath 源文件路径 @return true=成功 */
bool DashboardModel::loadFromFile(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        return false;
    }
    if (!doc.isArray()) {
        return false;
    }

    m_configs.clear();
    const QJsonArray arr = doc.array();
    for (const auto &item : arr) {
        if (item.isObject()) {
            m_configs.append(item.toObject().toVariantMap());
        }
    }

    /* 统计：配置文件加载计数 */
    ++m_profileLoads;
    ++m_totalLayoutLoads;

    emit configChanged();
    return true;
}

/** @brief 添加数据通道，若名称已存在则更新其初始值，通道增减后发射channelsChanged信号 @param name 通道名称 @param initialValue 初始值，默认0.0 */
void DashboardModel::addChannel(const QString &name, double initialValue)
{
    m_channels.insert(name, initialValue);
    emit channelsChanged();
}

/** @brief 移除数据通道，通道不存在时无操作，通道增减后发射channelsChanged信号 @param name 通道名称 */
void DashboardModel::removeChannel(const QString &name)
{
    if (m_channels.remove(name) > 0) {
        emit channelsChanged();
    }
}

/** @brief 更新通道值并通知视图，值变化时发射valueChanged信号，通道不存在时无操作 @param name 通道名称 @param value 新值 */
void DashboardModel::updateValue(const QString &name, double value)
{
    auto it = m_channels.find(name);
    if (it == m_channels.end()) {
        return;
    }

    /* 更新值范围统计 */
    if (!m_channelMin.contains(name) || value < m_channelMin[name]) {
        m_channelMin[name] = value;
    }
    if (!m_channelMax.contains(name) || value > m_channelMax[name]) {
        m_channelMax[name] = value;
    }

    if (!qFuzzyCompare(it.value(), value)) {
        it.value() = value;
        ++m_channelChangeCount[name];
        ++m_totalUpdateCount;
        emit valueChanged(name, value);
    }
}

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
