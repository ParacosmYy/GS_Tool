/**
 * @file algo_1912.cpp
 * @brief Algorithm module 1912
 */
#include "compress1912/algo_1912.h"
QVector<double> algo_1912::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
