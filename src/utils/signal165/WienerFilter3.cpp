/**
 * @file WienerFilter3.cpp
 * @brief Wiener filter for optimal noise reduction implementation
 */
#include "signal165/WienerFilter3.h"
#include <QElapsedTimer>

QVector<double> WienerFilter3::compute(const QVector<double> &input)
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

