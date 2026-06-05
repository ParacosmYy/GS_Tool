/**
 * @file algo_2543.cpp
 * @brief Algorithm module 2543
 */
#include "string2543/algo_2543.h"
QVector<double> algo_2543::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
