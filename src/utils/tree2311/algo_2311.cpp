/**
 * @file algo_2311.cpp
 * @brief Algorithm module 2311
 */
#include "tree2311/algo_2311.h"
QVector<double> algo_2311::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
