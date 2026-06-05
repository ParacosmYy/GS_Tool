/**
 * @file algo_1994.cpp
 * @brief Algorithm module 1994
 */
#include "numeric1994/algo_1994.h"
QVector<double> algo_1994::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
