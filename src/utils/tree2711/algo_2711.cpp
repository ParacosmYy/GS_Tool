/**
 * @file algo_2711.cpp
 * @brief Algorithm module 2711
 */
#include "tree2711/algo_2711.h"
QVector<double> algo_2711::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
