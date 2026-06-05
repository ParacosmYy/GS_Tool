/**
 * @file algo_2780.cpp
 * @brief Algorithm module 2780
 */
#include "sort2780/algo_2780.h"
QVector<double> algo_2780::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
