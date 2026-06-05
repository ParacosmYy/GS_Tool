/**
 * @file algo_983.cpp
 * @brief Algorithm module 983
 */
#include "string983/algo_983.h"
QVector<double> algo_983::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
