/**
 * @file algo_2330.cpp
 * @brief Algorithm module 2330
 */
#include "cluster2330/algo_2330.h"
QVector<double> algo_2330::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
