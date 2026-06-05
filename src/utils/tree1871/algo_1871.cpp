/**
 * @file algo_1871.cpp
 * @brief Algorithm module 1871
 */
#include "tree1871/algo_1871.h"
QVector<double> algo_1871::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
