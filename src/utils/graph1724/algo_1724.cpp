/**
 * @file algo_1724.cpp
 * @brief Algorithm module 1724
 */
#include "graph1724/algo_1724.h"
QVector<double> algo_1724::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
