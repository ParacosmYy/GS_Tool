/**
 * @file algo_993.cpp
 * @brief Algorithm module 993
 */
#include "crypto993/algo_993.h"
QVector<double> algo_993::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
