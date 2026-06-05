/**
 * @file algo_2172.cpp
 * @brief Algorithm module 2172
 */
#include "compress2172/algo_2172.h"
QVector<double> algo_2172::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
