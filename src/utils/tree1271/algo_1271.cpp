/**
 * @file algo_1271.cpp
 * @brief Algorithm module 1271
 */
#include "tree1271/algo_1271.h"
QVector<double> algo_1271::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
