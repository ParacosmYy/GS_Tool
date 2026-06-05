/**
 * @file algo_2603.cpp
 * @brief Algorithm module 2603
 */
#include "string2603/algo_2603.h"
QVector<double> algo_2603::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
