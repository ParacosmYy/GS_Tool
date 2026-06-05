/**
 * @file algo_1591.cpp
 * @brief Algorithm module 1591
 */
#include "tree1591/algo_1591.h"
QVector<double> algo_1591::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
