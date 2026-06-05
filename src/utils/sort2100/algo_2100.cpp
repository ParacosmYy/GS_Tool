/**
 * @file algo_2100.cpp
 * @brief Algorithm module 2100
 */
#include "sort2100/algo_2100.h"
QVector<double> algo_2100::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
