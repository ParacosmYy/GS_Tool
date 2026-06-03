/**
 * @file RecordingMarker.cpp
 * @brief 录制标记管理器实现
 *
 * 维护按时间戳升序排列的标记列表，支持有序插入、
 * 按索引删除、范围查询和最近标记查找。
 */

#include "core/recording/RecordingMarker.h"

#include <algorithm>
#include <QtMath>

// ============================================================================
// 构造 / 析构
// ============================================================================

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
RecordingMarker::RecordingMarker(QObject* parent)
    : QObject(parent)
{
}

// ============================================================================
// 标记增删
// ============================================================================

/**
 * @brief 添加标记（按时间戳升序插入）
 *
 * 使用 std::lower_bound 在有序列表中找到第一个时间戳 >= timestampMs 的位置，
 * 将新标记插入该位置以保持列表有序。
 *
 * @param label       标记标签
 * @param timestampMs 标记时间戳（毫秒）
 */
void RecordingMarker::addMarker(const QString& label, qint64 timestampMs)
{
    // 构造新条目
    MarkerEntry entry;
    entry.label       = label;
    entry.timestampMs = timestampMs;

    // 使用 lower_bound 查找有序插入位置
    auto it = std::lower_bound(
        m_markers.begin(), m_markers.end(), entry,
        [](const MarkerEntry& lhs, const MarkerEntry& rhs) {
            return lhs.timestampMs < rhs.timestampMs;
        });

    // 计算索引并插入
    int index = static_cast<int>(std::distance(m_markers.begin(), it));
    m_markers.insert(index, entry);

    // 统计：累计添加标记计数
    ++m_totalMarkersAdded;

    emit markerAdded(index, label);
}

/**
 * @brief 移除指定索引的标记
 *
 * 索引越界时静默返回，不抛异常。
 *
 * @param index 标记索引
 */
void RecordingMarker::removeMarker(int index)
{
    if (index < 0 || index >= m_markers.size()) {
        return;
    }
    m_markers.removeAt(index);

    // 统计：累计移除标记计数
    ++m_totalMarkersRemoved;

    emit markerRemoved(index);
}

// ============================================================================
// 标记查询
// ============================================================================

/**
 * @brief 获取所有标记（按时间戳升序）
 * @return 标记列表的副本
 */
QList<MarkerEntry> RecordingMarker::markers() const
{
    return m_markers;
}

/**
 * @brief 获取标记数量
 * @return 当前标记总数
 */
int RecordingMarker::count() const
{
    return m_markers.size();
}

/**
 * @brief 获取指定索引的标记
 *
 * 索引越界时返回默认构造的 MarkerEntry（空标签，时间戳为 0）。
 *
 * @param index 标记索引
 * @return 对应的标记条目
 */
MarkerEntry RecordingMarker::marker(int index) const
{
    if (index < 0 || index >= m_markers.size()) {
        return MarkerEntry{};
    }
    return m_markers.at(index);
}

/**
 * @brief 清除所有标记
 *
 * 逐个发射 markerRemoved 信号（从后往前移除以保持索引稳定），
 * 最后清空列表。监听方可通过信号感知每个标记的移除。
 */
void RecordingMarker::clear()
{
    // 从后往前移除，保持发射的索引与实际位置一致
    while (!m_markers.isEmpty()) {
        int lastIdx = m_markers.size() - 1;
        m_markers.removeAt(lastIdx);
        ++m_totalMarkersRemoved;
        emit markerRemoved(lastIdx);
    }
}

/**
 * @brief 查找距离给定时间戳最近的标记
 *
 * 利用列表有序特性，通过线性扫描找到距离目标最近的标记。
 * 列表为空时返回 -1。
 *
 * @param timestampMs 目标时间戳（毫秒）
 * @return 最近标记的索引，空列表返回 -1
 */
int RecordingMarker::findNearest(qint64 timestampMs) const
{
    if (m_markers.isEmpty()) {
        return -1;
    }

    // 使用 lower_bound 找到第一个 >= 目标的元素
    MarkerEntry target;
    target.timestampMs = timestampMs;
    target.label       = QString();

    auto upper = std::lower_bound(
        m_markers.begin(), m_markers.end(), target,
        [](const MarkerEntry& lhs, const MarkerEntry& rhs) {
            return lhs.timestampMs < rhs.timestampMs;
        });

    int upperIdx = static_cast<int>(std::distance(m_markers.begin(), upper));

    // 情况1：所有标记都小于目标 → 最后一个最近
    if (upperIdx >= m_markers.size()) {
        return m_markers.size() - 1;
    }

    // 情况2：第一个标记就 >= 目标 → 检查是否需要前一个
    if (upperIdx == 0) {
        return 0;
    }

    // 情况3：比较 upperIdx 和 upperIdx-1 哪个更近
    qint64 upperDiff = qAbs(m_markers[upperIdx].timestampMs - timestampMs);
    qint64 lowerDiff = qAbs(m_markers[upperIdx - 1].timestampMs - timestampMs);

    return (lowerDiff <= upperDiff) ? (upperIdx - 1) : upperIdx;
}

/**
 * @brief 查找指定时间范围内的所有标记
 *
 * 利用列表有序特性，从第一个 >= fromMs 的位置开始扫描，
 * 直到超出 toMs 为止。fromMs > toMs 时返回空列表。
 *
 * @param fromMs 起始时间戳（毫秒，含）
 * @param toMs   结束时间戳（毫秒，含）
 * @return 范围内标记的索引列表（按时间升序）
 */
QList<int> RecordingMarker::findInRange(qint64 fromMs, qint64 toMs) const
{
    QList<int> result;

    // 参数校验：范围无效
    if (fromMs > toMs) {
        return result;
    }

    if (m_markers.isEmpty()) {
        return result;
    }

    // 用 lower_bound 定位起始位置
    MarkerEntry target;
    target.timestampMs = fromMs;
    target.label       = QString();

    auto it = std::lower_bound(
        m_markers.begin(), m_markers.end(), target,
        [](const MarkerEntry& lhs, const MarkerEntry& rhs) {
            return lhs.timestampMs < rhs.timestampMs;
        });

    // 从起始位置线性扫描到超出范围
    int idx = static_cast<int>(std::distance(m_markers.begin(), it));
    for (; idx < m_markers.size(); ++idx) {
        if (m_markers[idx].timestampMs > toMs) {
            break;
        }
        result.append(idx);
    }

    return result;
}

// ============================================================================
// 跳转
// ============================================================================

/**
 * @brief 跳转到指定索引的标记
 *
 * 索引越界时返回 -1 且不发射信号。
 *
 * @param index 目标标记索引
 * @return 对应标记的时间戳，越界返回 -1
 */
qint64 RecordingMarker::jumpToMarker(int index)
{
    if (index < 0 || index >= m_markers.size()) {
        return -1;
    }

    // 统计：累计跳转事件计数
    ++m_totalJumpEvents;

    qint64 ts = m_markers[index].timestampMs;
    emit markerJumped(index, ts);
    return ts;
}

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计添加标记总数 */
quint64 RecordingMarker::totalMarkersAdded() const
{
    return m_totalMarkersAdded;
}

/** @brief 获取累计移除标记总数 */
quint64 RecordingMarker::totalMarkersRemoved() const
{
    return m_totalMarkersRemoved;
}

/** @brief 获取累计跳转事件总数 */
quint64 RecordingMarker::totalJumpEvents() const
{
    return m_totalJumpEvents;
}

/**
 * @brief 重置所有统计计数器为零
 */
void RecordingMarker::resetStats()
{
    m_totalMarkersAdded = 0;
    m_totalMarkersRemoved = 0;
    m_totalJumpEvents = 0;
}
