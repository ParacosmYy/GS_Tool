/**
 * @file algo_1991.cpp
 * @brief Algorithm module 1991
 */
#include "tree1991/algo_1991.h"
QVector<double> algo_1991::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
