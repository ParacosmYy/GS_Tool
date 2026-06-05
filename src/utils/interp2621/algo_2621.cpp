/**
 * @file algo_2621.cpp
 * @brief Algorithm module 2621
 */
#include "interp2621/algo_2621.h"
QVector<double> algo_2621::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
