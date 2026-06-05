/**
 * @file MemoryInspector.cpp
 * @brief Inspect and visualize memory layout of embedded targets implementation
 */
#include "memory/MemoryInspector.h"

MemoryInspector::MemoryInspector(QObject *parent)
    : QObject(parent)
{
}

MemoryInspector::~MemoryInspector() = default;

QByteArray MemoryInspector::process(const QByteArray &input)
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

void MemoryInspector::resetStatistics()
{
    m_stats = Stats{};
}

