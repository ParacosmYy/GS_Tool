/**
 * @file algo_1723.cpp
 * @brief Algorithm module 1723
 */
#include "string1723/algo_1723.h"
QVector<double> algo_1723::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
