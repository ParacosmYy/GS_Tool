/**
 * @file PhaseVocoder2.cpp
 * @brief Phase vocoder for time-stretching audio/data implementation
 */
#include "dsp163/PhaseVocoder2.h"
#include <QElapsedTimer>

QVector<double> PhaseVocoder2::compute(const QVector<double> &input)
{
    QElapsedTimer t;
    t.start();
    m_stats.calls++;

    if (input.isEmpty()) {
        m_stats.errors++;
        return {};
    }

    QVector<double> result = input;
    m_stats.itemsProcessed += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

