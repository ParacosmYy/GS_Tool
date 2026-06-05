/**
 * @file algo_2380.cpp
 * @brief Algorithm module 2380
 */
#include "sort2380/algo_2380.h"
QVector<double> algo_2380::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
