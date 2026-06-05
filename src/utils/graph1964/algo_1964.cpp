/**
 * @file algo_1964.cpp
 * @brief Algorithm module 1964
 */
#include "graph1964/algo_1964.h"
QVector<double> algo_1964::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
