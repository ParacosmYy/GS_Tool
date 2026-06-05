/**
 * @file algo_1310.cpp
 * @brief Algorithm module 1310
 */
#include "cluster1310/algo_1310.h"
QVector<double> algo_1310::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
