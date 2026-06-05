/**
 * @file algo_2060.cpp
 * @brief Algorithm module 2060
 */
#include "sort2060/algo_2060.h"
QVector<double> algo_2060::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
