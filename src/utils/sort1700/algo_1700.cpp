/**
 * @file algo_1700.cpp
 * @brief Algorithm module 1700
 */
#include "sort1700/algo_1700.h"
QVector<double> algo_1700::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
