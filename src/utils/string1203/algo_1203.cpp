/**
 * @file algo_1203.cpp
 * @brief Algorithm module 1203
 */
#include "string1203/algo_1203.h"
QVector<double> algo_1203::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
