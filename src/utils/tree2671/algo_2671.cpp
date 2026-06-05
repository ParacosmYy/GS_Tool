/**
 * @file algo_2671.cpp
 * @brief Algorithm module 2671
 */
#include "tree2671/algo_2671.h"
QVector<double> algo_2671::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
