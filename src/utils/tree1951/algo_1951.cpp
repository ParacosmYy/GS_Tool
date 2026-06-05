/**
 * @file algo_1951.cpp
 * @brief Algorithm module 1951
 */
#include "tree1951/algo_1951.h"
QVector<double> algo_1951::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
