/**
 * @file algo_2590.cpp
 * @brief Algorithm module 2590
 */
#include "cluster2590/algo_2590.h"
QVector<double> algo_2590::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
