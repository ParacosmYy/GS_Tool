/**
 * @file algo_1883.cpp
 * @brief Algorithm module 1883
 */
#include "string1883/algo_1883.h"
QVector<double> algo_1883::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
