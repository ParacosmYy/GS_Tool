/**
 * @file algo_1481.cpp
 * @brief Algorithm module 1481
 */
#include "interp1481/algo_1481.h"
QVector<double> algo_1481::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
