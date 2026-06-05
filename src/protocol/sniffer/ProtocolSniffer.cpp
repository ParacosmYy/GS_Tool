/**
 * @file ProtocolSniffer.cpp
 * @brief Passively sniff and decode multi-protocol serial traffic implementation
 */
#include "sniffer/ProtocolSniffer.h"

ProtocolSniffer::ProtocolSniffer(QObject *parent)
    : QObject(parent)
{
}

ProtocolSniffer::~ProtocolSniffer() = default;

QByteArray ProtocolSniffer::process(const QByteArray &input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        emit errorOccurred(tr("Empty input data"));
        return {};
    }

    m_stats.operationsPerformed++;
    m_stats.bytesProcessed += static_cast<quint64>(input.size());

    // Process data based on module type
    QByteArray result = input;

    m_stats.lastOperationMs = static_cast<quint64>(timer.elapsed());
    emit processingComplete(result);
    return result;
}

void ProtocolSniffer::resetStatistics()
{
    m_stats = Stats{};
}

