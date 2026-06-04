/**
 * @file ScopeChannelManager.cpp
 * @brief 示波器通道管理器实现 — 通道CRUD/自动缩放/JSON导入导出
 */
#include "widgets/scope/ScopeChannelManager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSaveFile>
#include <algorithm>
#include <QtMath>

// ── 构造函数 ──

/**
 * @brief 构造函数，初始化通道管理器
 * @param maxChannels 最大通道数量上限(默认8)
 * @param parent 父QObject
 */
ScopeChannelManager::ScopeChannelManager(int maxChannels, QObject* parent)
    : QObject(parent)
    , m_maxChannels(qMax(1, maxChannels))
    , m_nextIndex(0)
{
}

// ── 通道 CRUD ──

/**
 * @brief 添加一个新通道
 *
 * 忽略config中的index字段，由管理器内部递增分配唯─索引。
 * 若当前通道数已达上限则返回-1。
 * @param config 通道初始配置(index字段被忽略)
 * @return 分配到的通道索引，失败返回-1
 */
int ScopeChannelManager::addChannel(const ChannelConfig& config)
{
    if (m_channels.size() >= m_maxChannels) {
        return -1;
    }

    ChannelConfig cfg = config;
    cfg.index = m_nextIndex++;
    m_channels.insert(cfg.index, cfg);

    // 更新统计
    ++m_stats.totalChannelAdds;
    ++m_stats.activeChannelCount;
    if (m_stats.activeChannelCount > m_stats.peakChannelCount) {
        m_stats.peakChannelCount = m_stats.activeChannelCount;
    }

    emit channelAdded(cfg.index);
    return cfg.index;
}

/**
 * @brief 移除指定索引的通道
 * @param index 要移除的通道索引
 * @return 成功返回true，通道不存在返回false
 */
bool ScopeChannelManager::removeChannel(int index)
{
    if (!m_channels.contains(index)) {
        return false;
    }

    m_channels.remove(index);

    // 更新统计
    ++m_stats.totalChannelRemoves;
    --m_stats.activeChannelCount;

    emit channelRemoved(index);
    return true;
}

/**
 * @brief 更新指定通道的完整配置
 *
 * 会保留管理器分配的原始index，仅更新其他字段。
 * @param index 通道索引
 * @param config 新的通道配置(其中index字段被忽略)
 * @return 成功返回true，通道不存在返回false
 */
bool ScopeChannelManager::updateChannel(int index, const ChannelConfig& config)
{
    if (!m_channels.contains(index)) {
        return false;
    }

    ChannelConfig cfg = config;
    cfg.index = index; // 保留原始索引，防止外部篡改
    m_channels[index] = cfg;

    emit channelUpdated(index);
    return true;
}

/**
 * @brief 获取指定通道的配置副本
 * @param index 通道索引
 * @return 通道配置副本，索引不存在时返回默认配置(index=-1)
 */
ScopeChannelManager::ChannelConfig ScopeChannelManager::channel(int index) const
{
    return m_channels.value(index);
}

/**
 * @brief 获取所有通道配置(按索引升序排列)
 * @return 通道配置列表
 */
QList<ScopeChannelManager::ChannelConfig> ScopeChannelManager::channels() const
{
    QList<ChannelConfig> result;
    result.reserve(m_channels.size());
    for (auto it = m_channels.constBegin(); it != m_channels.constEnd(); ++it) {
        result.append(it.value());
    }
    return result;
}

/**
 * @brief 获取当前通道总数
 * @return 活跃通道数量
 */
int ScopeChannelManager::channelCount() const
{
    return m_channels.size();
}

/**
 * @brief 获取最大通道数上限
 * @return 最大通道数
 */
int ScopeChannelManager::maxChannels() const
{
    return m_maxChannels;
}

// ── 通道属性设置 ──

/**
 * @brief 设置通道可见性
 * @param index 通道索引
 * @param visible 是否可见
 */
void ScopeChannelManager::setChannelVisible(int index, bool visible)
{
    if (!m_channels.contains(index)) {
        return;
    }

    ChannelConfig& cfg = m_channels[index];
    if (cfg.visible != visible) {
        cfg.visible = visible;
        ++m_stats.totalVisibilityToggles;
        emit channelVisibilityChanged(index, visible);
    }
}

/**
 * @brief 设置通道波形颜色
 * @param index 通道索引
 * @param color 新颜色
 */
void ScopeChannelManager::setChannelColor(int index, const QColor& color)
{
    if (!m_channels.contains(index)) {
        return;
    }

    ChannelConfig& cfg = m_channels[index];
    if (cfg.color != color) {
        cfg.color = color;
        ++m_stats.totalColorChanges;
        emit channelUpdated(index);
    }
}

/**
 * @brief 设置通道Y轴范围
 *
 * 若yMin >= yMax则忽略此次设置，避免无效范围导致渲染异常。
 * @param index 通道索引
 * @param yMin Y轴下限
 * @param yMax Y轴上限
 */
void ScopeChannelManager::setChannelRange(int index, double yMin, double yMax)
{
    if (!m_channels.contains(index)) {
        return;
    }

    // 防御: 拒绝无效范围(下限必须严格小于上限)
    if (yMin >= yMax) {
        return;
    }

    ChannelConfig& cfg = m_channels[index];
    bool changed = !qFuzzyCompare(cfg.yMin, yMin) || !qFuzzyCompare(cfg.yMax, yMax);
    if (changed) {
        cfg.yMin = yMin;
        cfg.yMax = yMax;
        cfg.scaleMode = ScaleMode::Manual; // 手动设置范围时切换为手动模式
        ++m_stats.totalRangeChanges;
        emit channelRangeChanged(index, yMin, yMax);
    }
}

/**
 * @brief 设置通道物理单位
 * @param index 通道索引
 * @param unit 单位字符串(如 "V", "A", "mV")
 */
void ScopeChannelManager::setChannelUnit(int index, const QString& unit)
{
    if (!m_channels.contains(index)) {
        return;
    }

    ChannelConfig& cfg = m_channels[index];
    if (cfg.unit != unit) {
        cfg.unit = unit;
        emit channelUpdated(index);
    }
}

// ── 自动缩放 ──

/**
 * @brief 根据数据自动缩放指定通道Y轴范围
 *
 * 计算数据的实际最小值和最大值，然后各方向扩展10%作为边距。
 * 空数据或数据范围为零时，范围设为 [-1, 1] 作为安全默认值。
 * @param index 通道索引
 * @param data 采样数据向量
 */
void ScopeChannelManager::autoScale(int index, const QVector<double>& data)
{
    if (!m_channels.contains(index) || data.isEmpty()) {
        return;
    }

    // 计算数据实际最小值和最大值
    double dataMin = data.first();
    double dataMax = data.first();
    for (const double v : data) {
        if (v < dataMin) dataMin = v;
        if (v > dataMax) dataMax = v;
    }

    double range = dataMax - dataMin;

    // 数据范围极小时使用安全默认值
    if (qFuzzyIsNull(range)) {
        dataMin -= 1.0;
        dataMax += 1.0;
    } else {
        // 10%边距: 上下各扩展数据范围的10%
        double margin = range * 0.1;
        dataMin -= margin;
        dataMax += margin;
    }

    ChannelConfig& cfg = m_channels[index];
    cfg.yMin = dataMin;
    cfg.yMax = dataMax;
    cfg.scaleMode = ScaleMode::Auto;

    ++m_stats.totalScaleOperations;
    ++m_stats.totalRangeChanges;
    emit channelRangeChanged(index, dataMin, dataMax);
}

/**
 * @brief 对多个通道批量自动缩放
 * @param data 通道索引→采样数据的映射表
 */
void ScopeChannelManager::autoScaleAll(const QMap<int, QVector<double>>& data)
{
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        autoScale(it.key(), it.value());
    }
}

/**
 * @brief 重置所有通道的Y轴偏移为零
 */
void ScopeChannelManager::resetAllOffsets()
{
    for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
        if (!qFuzzyIsNull(it.value().offset)) {
            it.value().offset = 0.0;
            emit channelUpdated(it.key());
        }
    }
}

/**
 * @brief 获取所有可见通道的索引列表(按索引升序)
 * @return 可见通道索引列表
 */
QList<int> ScopeChannelManager::visibleChannels() const
{
    QList<int> result;
    for (auto it = m_channels.constBegin(); it != m_channels.constEnd(); ++it) {
        if (it.value().visible) {
            result.append(it.key());
        }
    }
    return result;
}

// ── JSON 导入/导出 ──

/**
 * @brief 将通道配置序列化为JSON并写入文件
 *
 * JSON格式:
 * {
 *   "version": 1,
 *   "maxChannels": 8,
 *   "channels": [
 *     { "index": 0, "name": "CH1", "color": "#00ff00", "yMin": -5.0,
 *       "yMax": 5.0, "unit": "V", "coupling": 0, "scaleMode": 0,
 *       "visible": true, "inverted": false, "probeRatio": 1.0,
 *       "offset": 0.0, "lineWidth": 2 },
 *     ...
 *   ]
 * }
 * @param filePath 目标文件路径
 * @return 成功返回true
 */
bool ScopeChannelManager::exportConfig(const QString& filePath)
{
    QJsonObject root;
    root["version"] = 1;
    root["maxChannels"] = m_maxChannels;

    QJsonArray chArray;
    for (auto it = m_channels.constBegin(); it != m_channels.constEnd(); ++it) {
        const ChannelConfig& cfg = it.value();
        QJsonObject obj;
        obj["index"] = cfg.index;
        obj["name"] = cfg.name;
        obj["color"] = cfg.color.name(QColor::HexArgb);
        obj["yMin"] = cfg.yMin;
        obj["yMax"] = cfg.yMax;
        obj["unit"] = cfg.unit;
        obj["coupling"] = static_cast<int>(cfg.coupling);
        obj["scaleMode"] = static_cast<int>(cfg.scaleMode);
        obj["visible"] = cfg.visible;
        obj["inverted"] = cfg.inverted;
        obj["probeRatio"] = cfg.probeRatio;
        obj["offset"] = cfg.offset;
        obj["lineWidth"] = cfg.lineWidth;
        chArray.append(obj);
    }
    root["channels"] = chArray;

    QJsonDocument doc(root);

    // 使用QSaveFile保证原子写入: 写入成功后才替换原文件
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}

/**
 * @brief 从JSON文件读取通道配置并追加到当前通道列表
 *
 * 解析JSON中的通道数组，逐个添加到管理器中。
 * 任何单通道解析失败仅跳过该通道，不影响其余通道。
 * @param filePath 源JSON文件路径
 * @return 至少成功导入一个通道返回true
 */
bool ScopeChannelManager::importConfig(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray chArray = root["channels"].toArray();

    bool anySuccess = false;
    for (const QJsonValue& val : chArray) {
        QJsonObject obj = val.toObject();

        ChannelConfig cfg;
        cfg.name = obj["name"].toString();
        cfg.color = QColor(obj["color"].toString("#00FF00"));
        cfg.yMin = obj["yMin"].toDouble(-5.0);
        cfg.yMax = obj["yMax"].toDouble(5.0);
        cfg.unit = obj["unit"].toString("V");
        cfg.coupling = static_cast<CouplingMode>(obj["coupling"].toInt(0));
        cfg.scaleMode = static_cast<ScaleMode>(obj["scaleMode"].toInt(0));
        cfg.visible = obj["visible"].toBool(true);
        cfg.inverted = obj["inverted"].toBool(false);
        cfg.probeRatio = obj["probeRatio"].toDouble(1.0);
        cfg.offset = obj["offset"].toDouble(0.0);
        cfg.lineWidth = obj["lineWidth"].toInt(2);

        // 跳过无效范围
        if (cfg.yMin >= cfg.yMax) {
            continue;
        }
        // 跳过无效探头比
        if (cfg.probeRatio <= 0.0) {
            continue;
        }

        int assignedIndex = addChannel(cfg);
        if (assignedIndex >= 0) {
            anySuccess = true;
        }
    }

    return anySuccess;
}

// ── 统计 ──

/**
 * @brief 获取统计数据的只读引用
 * @return Stats常量引用
 */
const ScopeChannelManager::Stats& ScopeChannelManager::stats() const
{
    return m_stats;
}
