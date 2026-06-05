/**
 * @file algo_1060.cpp
 * @brief Algorithm module 1060
 */
#include "sort1060/algo_1060.h"
QVector<double> algo_1060::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
