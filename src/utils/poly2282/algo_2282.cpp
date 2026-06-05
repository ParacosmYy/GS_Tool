/**
 * @file algo_2282.cpp
 * @brief Algorithm module 2282
 */
#include "poly2282/algo_2282.h"
QVector<double> algo_2282::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
