/**
 * @file algo_1651.cpp
 * @brief Algorithm module 1651
 */
#include "tree1651/algo_1651.h"
QVector<double> algo_1651::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
