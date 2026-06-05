/**
 * @file algo_2651.cpp
 * @brief Algorithm module 2651
 */
#include "tree2651/algo_2651.h"
QVector<double> algo_2651::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
