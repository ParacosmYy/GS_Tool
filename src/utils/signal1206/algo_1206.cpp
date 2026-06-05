/**
 * @file algo_1206.cpp
 * @brief Algorithm module 1206
 */
#include "signal1206/algo_1206.h"
QVector<double> algo_1206::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
