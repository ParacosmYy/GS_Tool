/**
 * @file algo_1120.cpp
 * @brief Algorithm module 1120
 */
#include "sort1120/algo_1120.h"
QVector<double> algo_1120::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
