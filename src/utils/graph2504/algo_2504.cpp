/**
 * @file algo_2504.cpp
 * @brief Algorithm module 2504
 */
#include "graph2504/algo_2504.h"
QVector<double> algo_2504::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
