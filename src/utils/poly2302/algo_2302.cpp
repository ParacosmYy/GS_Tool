/**
 * @file algo_2302.cpp
 * @brief Algorithm module 2302
 */
#include "poly2302/algo_2302.h"
QVector<double> algo_2302::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
