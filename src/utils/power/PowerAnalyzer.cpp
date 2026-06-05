/**
 * @file PowerAnalyzer.cpp
 * @brief Analyze power consumption patterns from current/voltage samples implementation
 */
#include "power/PowerAnalyzer.h"

PowerAnalyzer::PowerAnalyzer(QObject *parent)
    : QObject(parent)
{
}

PowerAnalyzer::~PowerAnalyzer() = default;

QByteArray PowerAnalyzer::process(const QByteArray &input)
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

void PowerAnalyzer::resetStatistics()
{
    m_stats = Stats{};
}

