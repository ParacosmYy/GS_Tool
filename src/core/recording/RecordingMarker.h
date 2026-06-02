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

private:
    QList<MarkerEntry> m_markers;  ///< 标记列表
};

#endif // RECORDING_MARKER_H
