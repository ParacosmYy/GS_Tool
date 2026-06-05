/**
 * @file algo_1380.cpp
 * @brief Algorithm module 1380
 */
#include "sort1380/algo_1380.h"
QVector<double> algo_1380::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
