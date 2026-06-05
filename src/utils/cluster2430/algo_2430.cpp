/**
 * @file algo_2430.cpp
 * @brief Algorithm module 2430
 */
#include "cluster2430/algo_2430.h"
QVector<double> algo_2430::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
