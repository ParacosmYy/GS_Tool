/**
 * @file algo_1800.cpp
 * @brief Algorithm module 1800
 */
#include "sort1800/algo_1800.h"
QVector<double> algo_1800::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
