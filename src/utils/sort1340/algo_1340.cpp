/**
 * @file algo_1340.cpp
 * @brief Algorithm module 1340
 */
#include "sort1340/algo_1340.h"
QVector<double> algo_1340::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
