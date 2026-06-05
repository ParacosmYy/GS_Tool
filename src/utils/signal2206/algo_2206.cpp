/**
 * @file algo_2206.cpp
 * @brief Algorithm module 2206
 */
#include "signal2206/algo_2206.h"
QVector<double> algo_2206::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
