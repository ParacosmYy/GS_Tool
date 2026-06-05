/**
 * @file algo_1980.cpp
 * @brief Algorithm module 1980
 */
#include "sort1980/algo_1980.h"
QVector<double> algo_1980::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
