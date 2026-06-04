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
#include <QSet>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造函数 @param parent 父对象指针 */
RecordingMarker::RecordingMarker(QObject* parent)
    : QObject(parent)
{
}

// ============================================================================
// 标记增删
// ============================================================================

/** @brief 添加标记(按时间戳升序插入，使用std::lower_bound保持有序) @param label 标记标签 @param timestampMs 标记时间戳(毫秒) */
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

/** @brief 移除指定索引的标记(越界时静默返回) @param index 标记索引 */
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

// 查询/导航/统计接口见 RecordingMarkerQuery.cpp
