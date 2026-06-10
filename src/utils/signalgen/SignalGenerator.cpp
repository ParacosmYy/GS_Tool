/**
 * @file SignalGenerator.cpp
 * @brief Generate test signals for protocol verification implementation
 */
#include "utils/signalgen/SignalGenerator.h"

#include <QElapsedTimer>

SignalGenerator::SignalGenerator(QObject *parent)
    : QObject(parent)
{
}

SignalGenerator::~SignalGenerator() = default;

QByteArray SignalGenerator::process(const QByteArray &input)
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

void SignalGenerator::resetStatistics()
{
    m_stats = Stats{};
}

