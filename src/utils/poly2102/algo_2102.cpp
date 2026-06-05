/**
 * @file algo_2102.cpp
 * @brief Algorithm module 2102
 */
#include "poly2102/algo_2102.h"
QVector<double> algo_2102::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
