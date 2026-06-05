/**
 * @file algo_1311.cpp
 * @brief Algorithm module 1311
 */
#include "tree1311/algo_1311.h"
QVector<double> algo_1311::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
