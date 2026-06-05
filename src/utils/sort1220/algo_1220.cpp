/**
 * @file algo_1220.cpp
 * @brief Algorithm module 1220
 */
#include "sort1220/algo_1220.h"
QVector<double> algo_1220::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
