/**
 * @file algo_2124.cpp
 * @brief Algorithm module 2124
 */
#include "graph2124/algo_2124.h"
QVector<double> algo_2124::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
