/**
 * @file algo_1180.cpp
 * @brief Algorithm module 1180
 */
#include "sort1180/algo_1180.h"
QVector<double> algo_1180::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
