/**
 * @file algo_2790.cpp
 * @brief Algorithm module 2790
 */
#include "cluster2790/algo_2790.h"
QVector<double> algo_2790::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
