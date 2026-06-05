/**
 * @file algo_2360.cpp
 * @brief Algorithm module 2360
 */
#include "sort2360/algo_2360.h"
QVector<double> algo_2360::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
