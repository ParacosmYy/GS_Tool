/**
 * @file algo_1953.cpp
 * @brief Algorithm module 1953
 */
#include "crypto1953/algo_1953.h"
QVector<double> algo_1953::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
