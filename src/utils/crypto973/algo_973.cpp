/**
 * @file algo_973.cpp
 * @brief Algorithm module 973
 */
#include "crypto973/algo_973.h"
QVector<double> algo_973::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
