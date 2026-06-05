/**
 * @file algo_2271.cpp
 * @brief Algorithm module 2271
 */
#include "tree2271/algo_2271.h"
QVector<double> algo_2271::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
