/**
 * @file algo_2111.cpp
 * @brief Algorithm module 2111
 */
#include "tree2111/algo_2111.h"
QVector<double> algo_2111::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
