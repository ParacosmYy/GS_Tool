/**
 * @file algo_2122.cpp
 * @brief Algorithm module 2122
 */
#include "poly2122/algo_2122.h"
QVector<double> algo_2122::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
