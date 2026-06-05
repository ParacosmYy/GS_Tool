/**
 * @file algo_2362.cpp
 * @brief Algorithm module 2362
 */
#include "poly2362/algo_2362.h"
QVector<double> algo_2362::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
