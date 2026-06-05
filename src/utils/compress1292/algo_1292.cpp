/**
 * @file algo_1292.cpp
 * @brief Algorithm module 1292
 */
#include "compress1292/algo_1292.h"
QVector<double> algo_1292::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
