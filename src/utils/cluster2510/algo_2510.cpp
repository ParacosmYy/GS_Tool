/**
 * @file algo_2510.cpp
 * @brief Algorithm module 2510
 */
#include "cluster2510/algo_2510.h"
QVector<double> algo_2510::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
