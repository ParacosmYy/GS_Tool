/**
 * @file algo_1900.cpp
 * @brief Algorithm module 1900
 */
#include "sort1900/algo_1900.h"
QVector<double> algo_1900::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
