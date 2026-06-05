/**
 * @file algo_1803.cpp
 * @brief Algorithm module 1803
 */
#include "string1803/algo_1803.h"
QVector<double> algo_1803::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
