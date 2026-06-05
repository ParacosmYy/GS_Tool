/**
 * @file algo_2181.cpp
 * @brief Algorithm module 2181
 */
#include "interp2181/algo_2181.h"
QVector<double> algo_2181::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
