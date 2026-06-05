/**
 * @file algo_2080.cpp
 * @brief Algorithm module 2080
 */
#include "sort2080/algo_2080.h"
QVector<double> algo_2080::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
