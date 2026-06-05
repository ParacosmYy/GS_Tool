/**
 * @file algo_1320.cpp
 * @brief Algorithm module 1320
 */
#include "sort1320/algo_1320.h"
QVector<double> algo_1320::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
