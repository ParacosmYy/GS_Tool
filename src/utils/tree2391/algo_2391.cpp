/**
 * @file algo_2391.cpp
 * @brief Algorithm module 2391
 */
#include "tree2391/algo_2391.h"
QVector<double> algo_2391::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
