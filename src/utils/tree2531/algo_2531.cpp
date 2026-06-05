/**
 * @file algo_2531.cpp
 * @brief Algorithm module 2531
 */
#include "tree2531/algo_2531.h"
QVector<double> algo_2531::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
