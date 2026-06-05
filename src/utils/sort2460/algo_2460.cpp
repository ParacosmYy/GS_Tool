/**
 * @file algo_2460.cpp
 * @brief Algorithm module 2460
 */
#include "sort2460/algo_2460.h"
QVector<double> algo_2460::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
