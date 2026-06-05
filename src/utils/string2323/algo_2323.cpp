/**
 * @file algo_2323.cpp
 * @brief Algorithm module 2323
 */
#include "string2323/algo_2323.h"
QVector<double> algo_2323::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
