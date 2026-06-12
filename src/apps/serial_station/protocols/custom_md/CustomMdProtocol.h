#ifndef CUSTOM_MD_PROTOCOL_H
#define CUSTOM_MD_PROTOCOL_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtCore/QVector>

#include "apps/serial_station/protocols/ISerialProtocol.h"

namespace serial_station {

/**
 * @brief MCU 自定义调试帧协议。
 *
 * 默认帧格式为 A5 5A | len | cmd | payload | checksum | 0D 0A。
 */
class CustomMdProtocol final : public ISerialProtocol {
public:
    QString name() const override;
    QByteArray buildCommand(const QString& command, const QVariantMap& params) const override;
    QVector<SerialProtocolEvent> feed(const QByteArray& data) override;
    void reset() override;

    QString lastError() const;

private:
    enum class ChecksumMode {
        Sum8,
        None
    };

    bool readCommandCode(const QString& command, quint8* commandCode) const;
    bool readPayload(const QVariantMap& params, QByteArray* payload) const;
    bool readBytesParam(const QVariantMap& params, const QString& key, QByteArray* bytes) const;
    bool readChecksumMode(const QVariantMap& params, ChecksumMode* mode) const;
    QByteArray buildFrame(
        const QByteArray& header,
        const QByteArray& footer,
        ChecksumMode mode,
        quint8 commandCode,
        const QByteArray& payload) const;
    quint8 checksumFor(const QByteArray& body) const;
    int findHeader() const;
    int minimumFrameLength() const;
    int expectedFrameLength() const;
    bool hasValidFooter(const QByteArray& frame) const;
    bool hasValidChecksum(const QByteArray& frame) const;
    SerialProtocolEvent makeFrameEvent(const QByteArray& frame) const;
    SerialProtocolEvent makeErrorEvent(const QString& message, const QByteArray& raw) const;
    void setError(const QString& message) const;

    QByteArray m_buffer;
    mutable QString m_lastError;
};

} // namespace serial_station

#endif // CUSTOM_MD_PROTOCOL_H
