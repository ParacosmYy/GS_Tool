/**
 * @file LegendrePoly4.cpp
 * @brief Legendre polynomial evaluation and root finding implementation
 */
#include "poly169/LegendrePoly4.h"
#include <QElapsedTimer>

QVector<double> LegendrePoly4::compute(const QVector<double> &input)
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

