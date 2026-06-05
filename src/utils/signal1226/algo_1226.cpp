/**
 * @file algo_1226.cpp
 * @brief Algorithm module 1226
 */
#include "signal1226/algo_1226.h"
QVector<double> algo_1226::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
