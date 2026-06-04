/** @file RecordingMarker.h @brief 录制标记管理器 - 在录制时间轴上添加/移除关键事件标记 */
#ifndef RECORDING_MARKER_H
#define RECORDING_MARKER_H

#include <QObject>
#include <QList>
#include <QString>

/** @brief 单个标记条目 */
struct MarkerEntry {
    QString label;        ///< 标记名称/描述
    qint64  timestampMs;  ///< 标记时间戳（毫秒）
};

/** @brief 录制标记管理类 - 维护有序标记列表，支持增删查/跳转/范围查找 */
class RecordingMarker : public QObject {
    Q_OBJECT

public:
    explicit RecordingMarker(QObject* parent = nullptr);

    void addMarker(const QString& label, qint64 timestampMs); ///< 添加标记
    void removeMarker(int index);                             ///< 移除指定索引标记
    QList<MarkerEntry> markers() const;                       ///< 获取所有标记
    int count() const;                                        ///< 获取标记数量
    MarkerEntry marker(int index) const;                      ///< 获取指定索引标记
    void clear();                                             ///< 清除所有标记
    int findNearest(qint64 timestampMs) const;                ///< 查找最近标记索引
    QList<int> findInRange(qint64 fromMs, qint64 toMs) const; ///< 查找时间范围内标记索引列表
    qint64 jumpToMarker(int index);                           ///< 跳转到指定标记(回放定位)

    // ---- 统计接口 ----
    quint64 totalMarkersCreated() const;
    quint64 totalMarkersAdded() const;
    quint64 totalMarkersRemoved() const;
    quint64 totalMarkersNavigated() const;
    quint64 totalJumpEvents() const;
    quint64 totalRangeQueries() const { return m_totalRangeQueries; }
    quint64 totalNearestQueries() const { return m_totalNearestQueries; }
    quint64 totalClearOps() const { return m_totalClearOps; }
    int markerTypesCount() const;
    void resetStats();

signals:
    void markerAdded(int index, const QString& label);       ///< 标记已添加信号
    void markerRemoved(int index);                           ///< 标记已移除信号
    void markerJumped(int index, qint64 timestampMs);        ///< 标记跳转信号

private:
    QList<MarkerEntry> m_markers;
    quint64 m_totalMarkersAdded = 0, m_totalMarkersRemoved = 0, m_totalJumpEvents = 0;
    mutable quint64 m_totalRangeQueries = 0, m_totalNearestQueries = 0;
    quint64 m_totalClearOps = 0;
};

#endif // RECORDING_MARKER_H
