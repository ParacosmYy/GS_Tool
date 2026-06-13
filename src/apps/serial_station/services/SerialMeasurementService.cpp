#include "apps/serial_station/services/SerialMeasurementService.h"

#include <QtCore/QVariantList>

namespace serial_station {

bool SerialMeasurementSnapshot::isEmpty() const
{
    return frameCount == 0 || channels.isEmpty();
}

bool SerialMeasurementService::appendEvent(const SerialProtocolEvent& event)
{
    if (event.type != QStringLiteral("measurement")) {
        return false;
    }

    const QVariantList values = event.payload.value(QStringLiteral("values")).toList();
    if (values.isEmpty()) {
        return false;
    }

    m_snapshot.protocolName = event.protocolName;
    ++m_snapshot.frameCount;
    for (int index = 0; index < values.size(); ++index) {
        updateChannel(index, values.at(index).toDouble());
    }
    return true;
}

void SerialMeasurementService::reset()
{
    m_snapshot = SerialMeasurementSnapshot();
}

SerialMeasurementSnapshot SerialMeasurementService::snapshot() const
{
    return m_snapshot;
}

QStringList SerialMeasurementService::displayLines() const
{
    QStringList lines;
    for (const SerialMeasurementChannel& channel : m_snapshot.channels) {
        lines.append(QStringLiteral("%1 latest=%2 samples=%3 min=%4 max=%5")
                         .arg(channel.name,
                              QString::number(channel.latest, 'g', 6),
                              QString::number(channel.sampleCount),
                              QString::number(channel.minimum, 'g', 6),
                              QString::number(channel.maximum, 'g', 6)));
    }
    return lines;
}

void SerialMeasurementService::updateChannel(int index, double value)
{
    while (m_snapshot.channels.size() <= index) {
        SerialMeasurementChannel channel;
        channel.index = m_snapshot.channels.size();
        channel.name = QStringLiteral("ch%1").arg(channel.index + 1);
        m_snapshot.channels.append(channel);
    }

    SerialMeasurementChannel& channel = m_snapshot.channels[index];
    if (channel.sampleCount == 0) {
        channel.minimum = value;
        channel.maximum = value;
    } else {
        channel.minimum = qMin(channel.minimum, value);
        channel.maximum = qMax(channel.maximum, value);
    }
    channel.latest = value;
    ++channel.sampleCount;
}

} // namespace serial_station
