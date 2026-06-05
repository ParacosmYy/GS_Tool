/**
 * @file algo_2662.cpp
 * @brief Algorithm module 2662
 */
#include "poly2662/algo_2662.h"
QVector<double> algo_2662::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
