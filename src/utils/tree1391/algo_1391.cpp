/**
 * @file algo_1391.cpp
 * @brief Algorithm module 1391
 */
#include "tree1391/algo_1391.h"
QVector<double> algo_1391::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
