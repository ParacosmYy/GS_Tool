/**
 * @file algo_2411.cpp
 * @brief Algorithm module 2411
 */
#include "tree2411/algo_2411.h"
QVector<double> algo_2411::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
