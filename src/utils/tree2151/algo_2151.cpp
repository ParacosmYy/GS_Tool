/**
 * @file algo_2151.cpp
 * @brief Algorithm module 2151
 */
#include "tree2151/algo_2151.h"
QVector<double> algo_2151::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
