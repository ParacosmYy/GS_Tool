/**
 * @file algo_2782.cpp
 * @brief Algorithm module 2782
 */
#include "poly2782/algo_2782.h"
QVector<double> algo_2782::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
