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
