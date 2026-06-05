/**
 * @file algo_1500.cpp
 * @brief Algorithm module 1500
 */
#include "sort1500/algo_1500.h"
QVector<double> algo_1500::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
