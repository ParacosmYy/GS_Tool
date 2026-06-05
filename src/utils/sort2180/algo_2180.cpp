/**
 * @file algo_2180.cpp
 * @brief Algorithm module 2180
 */
#include "sort2180/algo_2180.h"
QVector<double> algo_2180::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
