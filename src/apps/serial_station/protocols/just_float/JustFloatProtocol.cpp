#include "apps/serial_station/protocols/just_float/JustFloatProtocol.h"

#include <QtCore/QDataStream>
#include <QtCore/QIODevice>
#include <QtCore/QVariantList>

namespace serial_station {
namespace {

QByteArray justFloatTail()
{
    return QByteArray::fromHex("0000807F");
}

} // namespace

QString JustFloatProtocol::name() const
{
    return QStringLiteral("just_float");
}

QByteArray JustFloatProtocol::buildCommand(const QString& command,
                                           const QVariantMap& params) const
{
    QByteArray bytes = command.trimmed().toUtf8();
    if (params.value(QStringLiteral("appendNewline"), false).toBool()) {
        bytes.append('\n');
    }
    return bytes;
}

QVector<SerialProtocolEvent> JustFloatProtocol::feed(const QByteArray& data)
{
    QVector<SerialProtocolEvent> events;
    if (data.isEmpty()) {
        return events;
    }

    const QByteArray tail = justFloatTail();
    m_buffer.append(data);

    int tailIndex = m_buffer.indexOf(tail);
    while (tailIndex >= 0) {
        const int frameLength = tailIndex + tail.size();
        const QByteArray payload = m_buffer.left(tailIndex);
        const QByteArray rawFrame = m_buffer.left(frameLength);
        m_buffer.remove(0, frameLength);

        if (payload.isEmpty() || payload.size() % static_cast<int>(sizeof(float)) != 0) {
            events.append(buildErrorEvent(rawFrame, payload.size()));
        } else {
            events.append(buildMeasurementEvent(payload, rawFrame));
        }

        tailIndex = m_buffer.indexOf(tail);
    }

    return events;
}

void JustFloatProtocol::reset()
{
    m_buffer.clear();
    m_frameIndex = 0;
}

SerialProtocolEvent JustFloatProtocol::buildMeasurementEvent(const QByteArray& payload,
                                                             const QByteArray& rawFrame)
{
    QVariantList values;
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    while (!stream.atEnd()) {
        float value = 0.0F;
        stream >> value;
        values.append(static_cast<double>(value));
    }

    SerialProtocolEvent event;
    event.type = QStringLiteral("measurement");
    event.protocolName = name();
    event.raw = rawFrame;
    event.payload.insert(QStringLiteral("format"), name());
    event.payload.insert(QStringLiteral("values"), values);
    event.payload.insert(QStringLiteral("channelCount"), values.size());
    event.payload.insert(QStringLiteral("frameIndex"), ++m_frameIndex);
    return event;
}

SerialProtocolEvent JustFloatProtocol::buildErrorEvent(const QByteArray& rawFrame,
                                                       int payloadSize) const
{
    SerialProtocolEvent event;
    event.type = QStringLiteral("error");
    event.protocolName = name();
    event.raw = rawFrame;
    event.payload.insert(QStringLiteral("message"),
                         QStringLiteral("JustFloat payload length is invalid"));
    event.payload.insert(QStringLiteral("payloadSize"), payloadSize);
    return event;
}

} // namespace serial_station
