#include "apps/serial_station/services/SerialMeasurementService.h"

#include <QtCore/QStringList>
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
    appendRecentFrame(values);
    return true;
}

void SerialMeasurementService::setHistoryCapacity(int capacity)
{
    m_snapshot.historyCapacity = qMax(1, capacity);
    trimHistory();
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

QStringList SerialMeasurementService::trendLines() const
{
    QStringList lines;
    for (const SerialMeasurementFrame& frame : m_snapshot.recentFrames) {
        QStringList values;
        for (int index = 0; index < frame.values.size(); ++index) {
            values.append(QStringLiteral("ch%1=%2")
                              .arg(index + 1)
                              .arg(QString::number(frame.values.at(index), 'g', 6)));
        }
        lines.append(QStringLiteral("#%1 %2").arg(frame.frameIndex).arg(values.join(QLatin1Char(' '))));
    }
    return lines;
}

QString SerialMeasurementService::formatCsv() const
{
    if (m_snapshot.recentFrames.isEmpty()) {
        return QString();
    }

    int channelCount = 0;
    for (const SerialMeasurementFrame& frame : m_snapshot.recentFrames) {
        channelCount = qMax(channelCount, frame.values.size());
    }

    QStringList header;
    header.append(QStringLiteral("frame"));
    for (int index = 0; index < channelCount; ++index) {
        header.append(QStringLiteral("ch%1").arg(index + 1));
    }

    QStringList rows;
    rows.append(header.join(QLatin1Char(',')));
    for (const SerialMeasurementFrame& frame : m_snapshot.recentFrames) {
        QStringList row;
        row.append(QString::number(frame.frameIndex));
        for (int index = 0; index < channelCount; ++index) {
            row.append(index < frame.values.size()
                           ? QString::number(frame.values.at(index), 'g', 6)
                           : QString());
        }
        rows.append(row.join(QLatin1Char(',')));
    }
    return rows.join(QLatin1Char('\n')) + QLatin1Char('\n');
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

void SerialMeasurementService::appendRecentFrame(const QVariantList& values)
{
    SerialMeasurementFrame frame;
    frame.frameIndex = m_snapshot.frameCount;
    frame.values.reserve(values.size());
    for (const QVariant& value : values) {
        frame.values.append(value.toDouble());
    }
    m_snapshot.recentFrames.append(frame);
    trimHistory();
}

void SerialMeasurementService::trimHistory()
{
    while (m_snapshot.recentFrames.size() > m_snapshot.historyCapacity) {
        m_snapshot.recentFrames.removeFirst();
    }
}

} // namespace serial_station
