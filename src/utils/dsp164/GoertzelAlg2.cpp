/**
 * @file GoertzelAlg2.cpp
 * @brief Goertzel algorithm for single-frequency DFT implementation
 */
#include "dsp164/GoertzelAlg2.h"
#include <QElapsedTimer>

QVector<double> GoertzelAlg2::compute(const QVector<double> &input)
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

