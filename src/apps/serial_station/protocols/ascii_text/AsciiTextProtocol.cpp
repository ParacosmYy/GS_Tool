#include "apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h"

#include <QtCore/QStringList>

#include "apps/serial_station/SerialStationConstants.h"

namespace serial_station {

QString AsciiTextProtocol::name() const
{
    return serialStationConstants::kDefaultProtocolName;
}

QByteArray AsciiTextProtocol::buildCommand(const QString& command, const QVariantMap& params) const
{
    const QString text = params.value(QStringLiteral("text"), command).toString();
    QByteArray bytes = text.toUtf8();

    if (params.value(QStringLiteral("appendNewline"), false).toBool()) {
        bytes.append('\n');
    }

    return bytes;
}

QVector<SerialProtocolEvent> AsciiTextProtocol::feed(const QByteArray& data)
{
    QVector<SerialProtocolEvent> events;
    if (data.isEmpty()) {
        return events;
    }

    m_buffer.append(data);
    if (m_buffer.size() > serialStationConstants::kMaxProtocolBufferBytes) {
        m_buffer.clear();
        return events;
    }

    int newlineIndex = m_buffer.indexOf('\n');
    while (newlineIndex >= 0) {
        QByteArray rawLine = m_buffer.left(newlineIndex + 1);
        QByteArray payload = rawLine;
        if (payload.endsWith('\n')) {
            payload.chop(1);
        }
        if (payload.endsWith('\r')) {
            payload.chop(1);
        }

        SerialProtocolEvent event;
        event.type = serialStationConstants::kAsciiFrameType;
        event.protocolName = name();
        event.payload.insert(QStringLiteral("text"), QString::fromUtf8(payload));
        event.raw = rawLine;
        events.append(event);

        m_buffer.remove(0, newlineIndex + 1);
        newlineIndex = m_buffer.indexOf('\n');
    }

    return events;
}

void AsciiTextProtocol::reset()
{
    m_buffer.clear();
}

} // namespace serial_station
