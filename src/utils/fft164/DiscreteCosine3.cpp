/**
 * @file DiscreteCosine3.cpp
 * @brief Discrete cosine transform for signal compression implementation
 */
#include "fft164/DiscreteCosine3.h"
#include <QElapsedTimer>

QVector<double> DiscreteCosine3::compute(const QVector<double> &input)
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

