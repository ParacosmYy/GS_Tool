/**
 * @file algo_1993.cpp
 * @brief Algorithm module 1993
 */
#include "crypto1993/algo_1993.h"
QVector<double> algo_1993::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
