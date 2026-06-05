/**
 * @file algo_2450.cpp
 * @brief Algorithm module 2450
 */
#include "cluster2450/algo_2450.h"
QVector<double> algo_2450::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
