/**
 * @file DataBookmark.h
 * @brief 数据书签结构体 - 在录制时间轴上标记关键时间点
 *
 * 设计思路:
 *   DataBookmark 是一个纯数据结构，用于在录制数据流的时间轴上标记
 *   用户感兴趣的关键时刻（如异常发生、特定事件触发等）。
 *   支持通过 toJson()/fromJson() 进行 JSON 序列化，便于持久化存储。
 *   通过重载比较运算符支持按时间戳排序。
 *
 * 协作关系:
 *   - DataLogger: 持有 QVector<DataBookmark> 集合，提供增删查接口
 *   - RecordingController: 通过信号触发书签的添加
 *   - 未来 DataPlaybackWidget: 在时间轴上可视化书签标记
 */

#ifndef DATABOOKMARK_H
#define DATABOOKMARK_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QDateTime>

/**
 * @brief 数据书签 - 录制时间轴上的标记点
 *
 * 每个书签记录一个毫秒级精度的时间戳、用户定义的标签文本，
 * 以及可选的数据流标识（用于多流录制场景区分数据来源）。
 */
struct DataBookmark {
    /** @brief 书签时间戳（毫秒精度，Unix纪元时间，即 ms since epoch） */
    qint64 timestamp = 0;

    /** @brief 用户定义的书签标签，如 "异常发生"、"复位完成" 等 */
    QString label;

    /**
     * @brief 数据流标识，用于多流录制场景
     * 空字符串表示全局书签（不关联特定数据流）
     * 非空时标识具体的数据源，如 "serial"、"rtt" 等
     */
    QString streamId;

    /** @brief 默认构造 */
    DataBookmark() = default;

    /**
     * @brief 全参数构造
     * @param ts 时间戳（ms since epoch）
     * @param lbl 用户标签
     * @param sid 数据流标识（默认为空）
     */
    DataBookmark(qint64 ts, const QString& lbl, const QString& sid = QString())
        : timestamp(ts), label(lbl), streamId(sid) {}

    /**
     * @brief 序列化为 JSON 对象
     * @return QJsonObject 包含 timestamp/label/streamId 字段
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["timestamp"] = timestamp;
        obj["label"] = label;
        obj["streamId"] = streamId;
        return obj;
    }

    /**
     * @brief 从 JSON 对象反序列化
     * @param obj 包含 timestamp/label/streamId 的 JSON 对象
     * @return 反序列化后的 DataBookmark
     */
    static DataBookmark fromJson(const QJsonObject& obj) {
        DataBookmark bm;
        bm.timestamp = obj["timestamp"].toInteger(0);
        bm.label = obj["label"].toString();
        bm.streamId = obj["streamId"].toString();
        return bm;
    }

    /** @brief 小于比较 - 按时间戳升序排列 */
    bool operator<(const DataBookmark& other) const {
        return timestamp < other.timestamp;
    }

    /** @brief 相等比较 - 时间戳、标签、流ID 均相同 */
    bool operator==(const DataBookmark& other) const {
        return timestamp == other.timestamp
            && label == other.label
            && streamId == other.streamId;
    }
};

#endif // DATABOOKMARK_H
