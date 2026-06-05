/**
 * @file DBSCAN9.cpp
 * @brief Density-based spatial clustering with noise implementation
 */
#include "cluster162/DBSCAN9.h"
#include <QElapsedTimer>

QVector<double> DBSCAN9::compute(const QVector<double> &input)
{
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

