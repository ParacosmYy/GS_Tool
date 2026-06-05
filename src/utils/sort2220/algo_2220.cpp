/**
 * @file algo_2220.cpp
 * @brief Algorithm module 2220
 */
#include "sort2220/algo_2220.h"
QVector<double> algo_2220::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
