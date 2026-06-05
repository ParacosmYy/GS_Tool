/**
 * @file algo_2730.cpp
 * @brief Algorithm module 2730
 */
#include "cluster2730/algo_2730.h"
QVector<double> algo_2730::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
