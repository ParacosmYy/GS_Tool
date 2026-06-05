/**
 * @file EulerPath2.cpp
 * @brief Euler path/circuit detection in graphs implementation
 */
#include "graph178/EulerPath2.h"
#include <QElapsedTimer>

QVector<double> EulerPath2::compute(const QVector<double> &input)
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

