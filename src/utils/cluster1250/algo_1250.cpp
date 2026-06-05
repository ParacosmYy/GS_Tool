/**
 * @file algo_1250.cpp
 * @brief Algorithm module 1250
 */
#include "cluster1250/algo_1250.h"
QVector<double> algo_1250::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
