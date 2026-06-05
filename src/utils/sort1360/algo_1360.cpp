/**
 * @file algo_1360.cpp
 * @brief Algorithm module 1360
 */
#include "sort1360/algo_1360.h"
QVector<double> algo_1360::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
