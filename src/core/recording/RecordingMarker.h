/**
 * @file RecordingMarker.h
 * @brief 录制标记管理器，用于在录制时间轴上添加/移除关键事件标记
 *
 * 提供 F1 数据录制回放子系统中的事件标记功能，
 * 用户可在录制或回放过程中标记关键时间点。
 */

#ifndef RECORDING_MARKER_H
#define RECORDING_MARKER_H

#include <QObject>
#include <QList>
#include <QString>

/**
 * @struct MarkerEntry
 * @brief 单个标记条目
 */
struct MarkerEntry {
    QString label;        ///< 标记名称/描述
    qint64  timestampMs;  ///< 标记时间戳（毫秒）
};

/**
 * @class RecordingMarker
 * @brief 录制标记管理类
 *
 * 维护一个有序的标记列表，支持添加、删除和查询操作。
 * 每个标记包含一个标签和一个时间戳。
 */
class RecordingMarker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit RecordingMarker(QObject* parent = nullptr);

    /**
     * @brief 添加标记
     * @param label 标记标签
     * @param timestampMs 标记时间戳（毫秒）
     */
    void addMarker(const QString& label, qint64 timestampMs);

    /**
     * @brief 移除指定索引的标记
     * @param index 标记索引
     */
    void removeMarker(int index);

    /**
     * @brief 获取所有标记
     * @return 标记列表
     */
    QList<MarkerEntry> markers() const;

    /**
     * @brief 获取标记数量
     * @return 标记总数
     */
    int count() const;

    /**
     * @brief 获取指定索引的标记
     * @param index 标记索引
     * @return 对应的标记条目，越界时返回默认构造的 MarkerEntry
     */
    MarkerEntry marker(int index) const;

    /**
     * @brief 清除所有标记
     */
    void clear();

    /**
     * @brief 查找距离给定时间戳最近的标记
     * @param timestampMs 目标时间戳（毫秒）
     * @return 最近标记的索引，列表为空时返回 -1
     */
    int findNearest(qint64 timestampMs) const;

    /**
     * @brief 查找指定时间范围内的所有标记
     * @param fromMs 起始时间戳（毫秒，含）
     * @param toMs 结束时间戳（毫秒，含）
     * @return 范围内标记的索引列表
     */
    QList<int> findInRange(qint64 fromMs, qint64 toMs) const;

    /**
     * @brief 跳转到指定索引的标记（用于回放定位）
     * @param index 目标标记索引
     * @return 对应标记的时间戳，越界时返回 -1
     */
    qint64 jumpToMarker(int index);

    // ==================== 统计接口 ====================

    /** @brief 获取累计创建标记总数(同totalMarkersAdded) */
    quint64 totalMarkersCreated() const;

    /** @brief 获取累计添加标记总数 */
    quint64 totalMarkersAdded() const;

    /** @brief 获取累计移除标记总数 */
    quint64 totalMarkersRemoved() const;

    /** @brief 获取累计导航到标记的次数(同totalJumpEvents) */
    quint64 totalMarkersNavigated() const;

    /** @brief 获取累计跳转事件总数 */
    quint64 totalJumpEvents() const;

    /** @brief 获取累计范围查询次数 @return findInRange调用次数 */
    quint64 totalRangeQueries() const { return m_totalRangeQueries; }

    /** @brief 获取累计最近标记查询次数 @return findNearest调用次数 */
    quint64 totalNearestQueries() const { return m_totalNearestQueries; }

    /** @brief 获取累计清除操作次数 @return clear()调用次数 */
    quint64 totalClearOps() const { return m_totalClearOps; }

    /** @brief 获取当前不同标签类型的数量 */
    int markerTypesCount() const;

    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /**
     * @brief 标记已添加信号
     * @param index 新标记的索引
     * @param label 新标记的标签
     */
    void markerAdded(int index, const QString& label);

    /**
     * @brief 标记已移除信号
     * @param index 被移除标记的索引
     */
    void markerRemoved(int index);

    /**
     * @brief 标记跳转信号
     * @param index 跳转目标标记索引
     * @param timestampMs 跳转目标时间戳
     */
    void markerJumped(int index, qint64 timestampMs);

private:
    QList<MarkerEntry> m_markers;  ///< 标记列表

    // ---- 统计计数器 ----
    quint64 m_totalMarkersAdded = 0;    ///< 累计添加标记数
    quint64 m_totalMarkersRemoved = 0;  ///< 累计移除标记数
    quint64 m_totalJumpEvents = 0;      ///< 累计跳转事件数
    mutable quint64 m_totalRangeQueries = 0;    ///< 累计范围查询次数
    mutable quint64 m_totalNearestQueries = 0;  ///< 累计最近标记查询次数
    quint64 m_totalClearOps = 0;                ///< 累计清除操作次数
};

#endif // RECORDING_MARKER_H
