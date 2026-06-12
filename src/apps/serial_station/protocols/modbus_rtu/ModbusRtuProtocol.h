#ifndef MODBUS_RTU_PROTOCOL_H
#define MODBUS_RTU_PROTOCOL_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtCore/QVector>

#include "apps/serial_station/protocols/ISerialProtocol.h"

namespace serial_station {

/**
 * @brief Modbus RTU 主站协议。
 *
 * 协议层只负责 RTU 帧构建、CRC 校验和流式解析，不持有串口或 UI 对象。
 */
class ModbusRtuProtocol final : public ISerialProtocol {
public:
    QString name() const override;
    QByteArray buildCommand(const QString& command, const QVariantMap& params) const override;
    QVector<SerialProtocolEvent> feed(const QByteArray& data) override;
    void reset() override;

    QString lastError() const;

private:
    enum class ValueMode {
        Quantity,
        CoilValue,
        RegisterValue
    };

    QByteArray buildReadCommand(quint8 functionCode, const QVariantMap& params, int maxQuantity) const;
    QByteArray buildWriteCommand(quint8 functionCode, const QVariantMap& params, ValueMode mode) const;
    bool readByteParam(const QVariantMap& params, const QString& key, int minValue, int maxValue, quint8* value) const;
    bool readWordParam(const QVariantMap& params, const QString& key, int minValue, int maxValue, quint16* value) const;
    bool readCoilValue(const QVariantMap& params, quint16* value) const;
    QByteArray frameWithCrc(const QByteArray& frame) const;
    int expectedFrameLength() const;
    bool hasValidCrc(const QByteArray& frame) const;
    SerialProtocolEvent makeFrameEvent(const QByteArray& frame) const;
    SerialProtocolEvent makeErrorEvent(const QString& message, const QByteArray& raw) const;
    void setError(const QString& message) const;

    QByteArray m_buffer;
    mutable QString m_lastError;
};

} // namespace serial_station

#endif // MODBUS_RTU_PROTOCOL_H
