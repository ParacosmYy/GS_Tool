/**
 * @file algo_2704.cpp
 * @brief Algorithm module 2704
 */
#include "graph2704/algo_2704.h"
QVector<double> algo_2704::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
