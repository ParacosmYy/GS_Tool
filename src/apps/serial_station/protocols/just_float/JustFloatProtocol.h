#ifndef JUST_FLOAT_PROTOCOL_H
#define JUST_FLOAT_PROTOCOL_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "apps/serial_station/protocols/ISerialProtocol.h"

namespace serial_station {

/**
 * @brief VOFA+ JustFloat 浮点流协议。
 *
 * 接收侧解析小端 IEEE754 float 数组和固定帧尾，输出 measurement 事件。
 */
class JustFloatProtocol final : public ISerialProtocol {
public:
    QString name() const override;
    QByteArray buildCommand(const QString& command, const QVariantMap& params) const override;
    QVector<SerialProtocolEvent> feed(const QByteArray& data) override;
    void reset() override;

private:
    SerialProtocolEvent buildMeasurementEvent(const QByteArray& payload,
                                              const QByteArray& rawFrame);
    SerialProtocolEvent buildErrorEvent(const QByteArray& rawFrame,
                                        int payloadSize) const;

    QByteArray m_buffer;
    int m_frameIndex = 0;
};

} // namespace serial_station

#endif // JUST_FLOAT_PROTOCOL_H
