/**
 * @file algo_2004.cpp
 * @brief Algorithm module 2004
 */
#include "graph2004/algo_2004.h"
QVector<double> algo_2004::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
