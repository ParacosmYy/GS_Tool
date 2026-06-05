/**
 * @file algo_2340.cpp
 * @brief Algorithm module 2340
 */
#include "sort2340/algo_2340.h"
QVector<double> algo_2340::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
