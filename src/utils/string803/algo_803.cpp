/**
 * @file algo_803.cpp
 * @brief Algorithm module 803
 */
#include "string803/algo_803.h"
QVector<double> algo_803::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
