/**
 * @file algo_2020.cpp
 * @brief Algorithm module 2020
 */
#include "sort2020/algo_2020.h"
QVector<double> algo_2020::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
