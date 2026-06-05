/**
 * @file algo_991.cpp
 * @brief Algorithm module 991
 */
#include "tree991/algo_991.h"
QVector<double> algo_991::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
