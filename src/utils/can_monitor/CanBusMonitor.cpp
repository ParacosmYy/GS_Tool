/**
 * @file CanBusMonitor.cpp
 * @brief Monitor and decode CAN bus frames from serial adapter implementation
 */
#include "can_monitor/CanBusMonitor.h"

CanBusMonitor::CanBusMonitor(QObject *parent)
    : QObject(parent)
{
}

CanBusMonitor::~CanBusMonitor() = default;

QByteArray CanBusMonitor::process(const QByteArray &input)
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

void CanBusMonitor::resetStatistics()
{
    m_stats = Stats{};
}

