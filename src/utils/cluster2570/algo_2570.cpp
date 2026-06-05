/**
 * @file algo_2570.cpp
 * @brief Algorithm module 2570
 */
#include "cluster2570/algo_2570.h"
QVector<double> algo_2570::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
