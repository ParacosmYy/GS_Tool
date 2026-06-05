/**
 * @file algo_2062.cpp
 * @brief Algorithm module 2062
 */
#include "poly2062/algo_2062.h"
QVector<double> algo_2062::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
