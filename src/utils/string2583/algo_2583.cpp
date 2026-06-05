/**
 * @file algo_2583.cpp
 * @brief Algorithm module 2583
 */
#include "string2583/algo_2583.h"
QVector<double> algo_2583::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
