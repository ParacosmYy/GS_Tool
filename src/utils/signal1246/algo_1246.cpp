/**
 * @file algo_1246.cpp
 * @brief Algorithm module 1246
 */
#include "signal1246/algo_1246.h"
QVector<double> algo_1246::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
