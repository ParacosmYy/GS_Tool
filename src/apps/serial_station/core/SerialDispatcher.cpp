#include "apps/serial_station/core/SerialDispatcher.h"

#include <utility>

namespace serial_station {

void SerialDispatcher::setProtocol(std::unique_ptr<ISerialProtocol> protocol)
{
    m_protocol = std::move(protocol);
    reset();
    updateSummary(FeedStatus::EmptyInput, 0, 0);
}

bool SerialDispatcher::hasProtocol() const
{
    return m_protocol != nullptr;
}

QString SerialDispatcher::protocolName() const
{
    if (!m_protocol) {
        return {};
    }

    return m_protocol->name();
}

QVector<SerialProtocolEvent> SerialDispatcher::feed(const QByteArray& bytes)
{
    if (bytes.isEmpty()) {
        updateSummary(FeedStatus::EmptyInput, 0, 0);
        return {};
    }

    if (!m_protocol) {
        updateSummary(FeedStatus::MissingProtocol, bytes.size(), 0);
        return {};
    }

    QVector<SerialProtocolEvent> events = m_protocol->feed(bytes);
    if (events.isEmpty()) {
        updateSummary(FeedStatus::Buffered, bytes.size(), 0);
        return events;
    }

    updateSummary(FeedStatus::EventsReady, bytes.size(), events.count());
    return events;
}

void SerialDispatcher::reset()
{
    if (m_protocol) {
        m_protocol->reset();
    }
}

SerialDispatcher::FeedSummary SerialDispatcher::lastFeedSummary() const
{
    return m_lastSummary;
}

void SerialDispatcher::updateSummary(FeedStatus status, int inputBytes, int eventCount)
{
    m_lastSummary.status = status;
    m_lastSummary.inputBytes = inputBytes;
    m_lastSummary.eventCount = eventCount;
    m_lastSummary.protocolName = protocolName();
}

} // namespace serial_station
