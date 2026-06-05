/**
 * @file algo_1200.cpp
 * @brief Algorithm module 1200
 */
#include "sort1200/algo_1200.h"
QVector<double> algo_1200::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
