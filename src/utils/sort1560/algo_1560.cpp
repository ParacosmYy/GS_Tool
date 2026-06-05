/**
 * @file algo_1560.cpp
 * @brief Algorithm module 1560
 */
#include "sort1560/algo_1560.h"
QVector<double> algo_1560::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
