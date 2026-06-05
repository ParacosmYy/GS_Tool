/**
 * @file algo_1726.cpp
 * @brief Algorithm module 1726
 */
#include "signal1726/algo_1726.h"
QVector<double> algo_1726::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
