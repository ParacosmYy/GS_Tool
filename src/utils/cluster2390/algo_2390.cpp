/**
 * @file algo_2390.cpp
 * @brief Algorithm module 2390
 */
#include "cluster2390/algo_2390.h"
QVector<double> algo_2390::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
