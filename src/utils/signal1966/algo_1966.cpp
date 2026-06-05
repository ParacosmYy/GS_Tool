/**
 * @file algo_1966.cpp
 * @brief Algorithm module 1966
 */
#include "signal1966/algo_1966.h"
QVector<double> algo_1966::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
