/**
 * @file algo_1940.cpp
 * @brief Algorithm module 1940
 */
#include "sort1940/algo_1940.h"
QVector<double> algo_1940::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
