/**
 * @file algo_2602.cpp
 * @brief Algorithm module 2602
 */
#include "poly2602/algo_2602.h"
QVector<double> algo_2602::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
