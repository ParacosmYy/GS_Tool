/**
 * @file algo_1780.cpp
 * @brief Algorithm module 1780
 */
#include "sort1780/algo_1780.h"
QVector<double> algo_1780::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
