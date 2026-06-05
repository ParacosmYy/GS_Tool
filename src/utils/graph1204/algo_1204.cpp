/**
 * @file algo_1204.cpp
 * @brief Algorithm module 1204
 */
#include "graph1204/algo_1204.h"
QVector<double> algo_1204::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
