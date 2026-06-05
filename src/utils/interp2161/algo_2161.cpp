/**
 * @file algo_2161.cpp
 * @brief Algorithm module 2161
 */
#include "interp2161/algo_2161.h"
QVector<double> algo_2161::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
