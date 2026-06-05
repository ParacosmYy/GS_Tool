/**
 * @file algo_811.cpp
 * @brief Algorithm module 811
 */
#include "tree811/algo_811.h"
QVector<double> algo_811::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
