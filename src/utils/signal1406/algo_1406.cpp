/**
 * @file algo_1406.cpp
 * @brief Algorithm module 1406
 */
#include "signal1406/algo_1406.h"
QVector<double> algo_1406::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
