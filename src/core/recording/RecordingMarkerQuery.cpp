/**
 * @file RecordingMarkerQuery.cpp
 * @brief 录制标记管理器 - 查询与导航接口实现
 *
 * 从 RecordingMarker.cpp 拆分而来，包含标记查询(获取/计数/
 * 清除)、二分查找(最近/范围查询)、跳转导航和统计接口。
 */

#include "core/recording/RecordingMarker.h"

#include <algorithm>
#include <QtMath>
#include <QSet>

/** @brief 获取所有标记（按时间戳升序） @return 标记列表的副本 */
QList<MarkerEntry> RecordingMarker::markers() const
{
    return m_markers;
}

/** @brief 获取标记数量 @return 当前标记总数 */
int RecordingMarker::count() const
{
    return m_markers.size();
}

/** @brief 获取指定索引的标记(越界返回默认MarkerEntry) @param index 标记索引 @return 对应的标记条目 */
MarkerEntry RecordingMarker::marker(int index) const
{
    if (index < 0 || index >= m_markers.size()) {
        return MarkerEntry{};
    }
    return m_markers.at(index);
}

/** @brief 清除所有标记(从后往前移除，逐个发射markerRemoved信号) */
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

/** @brief 查找距离给定时间戳最近的标记(利用有序特性) @param timestampMs 目标时间戳(毫秒) @return 最近标记的索引，空列表返回-1 */
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

/** @brief 查找指定时间范围内的所有标记(利用有序特性) @param fromMs 起始时间戳(毫秒，含) @param toMs 结束时间戳(毫秒，含) @return 范围内标记的索引列表(按时间升序) */
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

/** @brief 跳转到指定索引的标记(越界返回-1且不发射信号) @param index 目标标记索引 @return 对应标记的时间戳，越界返回 -1 */
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

/** @brief 获取累计创建标记总数(同totalMarkersAdded) */
quint64 RecordingMarker::totalMarkersCreated() const
{
    return m_totalMarkersAdded;
}

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

/** @brief 获取累计导航到标记的次数(同totalJumpEvents) */
quint64 RecordingMarker::totalMarkersNavigated() const
{
    return m_totalJumpEvents;
}

/** @brief 获取当前不同标签类型的数量 */
int RecordingMarker::markerTypesCount() const
{
    QSet<QString> uniqueLabels;
    for (const auto& marker : m_markers) {
        uniqueLabels.insert(marker.label);
    }
    return uniqueLabels.size();
}

/** @brief 重置所有统计计数器为零 */
void RecordingMarker::resetStats()
{
    m_totalMarkersAdded = 0;
    m_totalMarkersRemoved = 0;
    m_totalJumpEvents = 0;
}
