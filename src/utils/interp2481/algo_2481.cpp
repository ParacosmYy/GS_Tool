/**
 * @file algo_2481.cpp
 * @brief Algorithm module 2481
 */
#include "interp2481/algo_2481.h"
QVector<double> algo_2481::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
