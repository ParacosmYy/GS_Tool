/**
 * @file algo_1483.cpp
 * @brief Algorithm module 1483
 */
#include "string1483/algo_1483.h"
QVector<double> algo_1483::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
