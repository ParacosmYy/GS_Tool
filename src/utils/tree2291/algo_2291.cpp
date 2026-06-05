/**
 * @file algo_2291.cpp
 * @brief Algorithm module 2291
 */
#include "tree2291/algo_2291.h"
QVector<double> algo_2291::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
