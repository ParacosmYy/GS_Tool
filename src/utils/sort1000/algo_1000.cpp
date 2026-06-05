/**
 * @file algo_1000.cpp
 * @brief Algorithm module 1000
 */
#include "sort1000/algo_1000.h"
QVector<double> algo_1000::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
