/**
 * @file algo_2404.cpp
 * @brief Algorithm module 2404
 */
#include "graph2404/algo_2404.h"
QVector<double> algo_2404::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
