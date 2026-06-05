/**
 * @file algo_2410.cpp
 * @brief Algorithm module 2410
 */
#include "cluster2410/algo_2410.h"
QVector<double> algo_2410::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
