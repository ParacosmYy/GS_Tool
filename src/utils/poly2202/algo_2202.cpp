/**
 * @file algo_2202.cpp
 * @brief Algorithm module 2202
 */
#include "poly2202/algo_2202.h"
QVector<double> algo_2202::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
