/**
 * @file algo_1846.cpp
 * @brief Algorithm module 1846
 */
#include "signal1846/algo_1846.h"
QVector<double> algo_1846::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
