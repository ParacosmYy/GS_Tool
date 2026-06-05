/**
 * @file algo_2101.cpp
 * @brief Algorithm module 2101
 */
#include "interp2101/algo_2101.h"
QVector<double> algo_2101::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
