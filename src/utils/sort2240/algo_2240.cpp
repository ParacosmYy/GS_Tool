/**
 * @file algo_2240.cpp
 * @brief Algorithm module 2240
 */
#include "sort2240/algo_2240.h"
QVector<double> algo_2240::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
