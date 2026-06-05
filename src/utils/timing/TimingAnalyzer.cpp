/**
 * @file TimingAnalyzer.cpp
 * @brief Measure and analyze timing characteristics of serial protocols implementation
 */
#include "timing/TimingAnalyzer.h"

TimingAnalyzer::TimingAnalyzer(QObject *parent)
    : QObject(parent)
{
}

TimingAnalyzer::~TimingAnalyzer() = default;

QByteArray TimingAnalyzer::process(const QByteArray &input)
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

void TimingAnalyzer::resetStatistics()
{
    m_stats = Stats{};
}

