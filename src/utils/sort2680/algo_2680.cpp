/**
 * @file algo_2680.cpp
 * @brief Algorithm module 2680
 */
#include "sort2680/algo_2680.h"
QVector<double> algo_2680::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
