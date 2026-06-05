/**
 * @file algo_2520.cpp
 * @brief Algorithm module 2520
 */
#include "sort2520/algo_2520.h"
QVector<double> algo_2520::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
