/**
 * @file algo_903.cpp
 * @brief Algorithm module 903
 */
#include "string903/algo_903.h"
QVector<double> algo_903::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
