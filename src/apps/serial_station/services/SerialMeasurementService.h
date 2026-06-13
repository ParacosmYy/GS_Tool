#ifndef SERIAL_MEASUREMENT_SERVICE_H
#define SERIAL_MEASUREMENT_SERVICE_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "apps/serial_station/protocols/SerialProtocolEvent.h"

namespace serial_station {

/** @brief 单个测量通道的会话内统计。 */
struct SerialMeasurementChannel {
    int index = 0;          ///< 从 0 开始的通道序号
    QString name;           ///< UI 展示名，默认 ch1/ch2
    double latest = 0.0;    ///< 最近一次测量值
    double minimum = 0.0;   ///< 会话内最小值
    double maximum = 0.0;   ///< 会话内最大值
    int sampleCount = 0;    ///< 当前通道累计样本数
};

/** @brief 一帧 measurement 事件的通道值快照。 */
struct SerialMeasurementFrame {
    int frameIndex = 0;      ///< 会话内测量帧序号，从 1 开始
    QVector<double> values;  ///< 该帧每个通道的测量值
};

/** @brief 当前测量会话快照。 */
struct SerialMeasurementSnapshot {
    QString protocolName;                       ///< 最近测量事件来源协议
    int frameCount = 0;                         ///< 累计测量帧数
    int historyCapacity = 120;                   ///< 最近帧缓冲容量
    QVector<SerialMeasurementChannel> channels; ///< 通道统计列表
    QVector<SerialMeasurementFrame> recentFrames; ///< 最近 measurement 帧

    /** @brief 是否尚未收到有效测量样本。 */
    bool isEmpty() const;
};

/**
 * @brief Serial Station 测量通道统计服务。
 *
 * 只处理协议层已经解析出的 measurement 事件，不接触 QWidget 或串口线程。
 */
class SerialMeasurementService {
public:
    bool appendEvent(const SerialProtocolEvent& event);
    void setHistoryCapacity(int capacity);
    void reset();
    SerialMeasurementSnapshot snapshot() const;
    QStringList displayLines() const;
    QStringList trendLines() const;
    QString formatCsv() const;

private:
    void updateChannel(int index, double value);
    void appendRecentFrame(const QVariantList& values);
    void trimHistory();

    SerialMeasurementSnapshot m_snapshot;
};

} // namespace serial_station

#endif // SERIAL_MEASUREMENT_SERVICE_H
