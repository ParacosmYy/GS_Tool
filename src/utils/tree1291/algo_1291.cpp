/**
 * @file algo_1291.cpp
 * @brief Algorithm module 1291
 */
#include "tree1291/algo_1291.h"
QVector<double> algo_1291::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
