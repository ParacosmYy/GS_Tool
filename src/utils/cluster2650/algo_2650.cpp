/**
 * @file algo_2650.cpp
 * @brief Algorithm module 2650
 */
#include "cluster2650/algo_2650.h"
QVector<double> algo_2650::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
