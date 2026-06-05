/**
 * @file algo_2523.cpp
 * @brief Algorithm module 2523
 */
#include "string2523/algo_2523.h"
QVector<double> algo_2523::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
