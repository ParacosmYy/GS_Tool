/**
 * @file algo_1086.cpp
 * @brief Algorithm module 1086
 */
#include "signal1086/algo_1086.h"
QVector<double> algo_1086::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
