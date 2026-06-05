/**
 * @file algo_1286.cpp
 * @brief Algorithm module 1286
 */
#include "signal1286/algo_1286.h"
QVector<double> algo_1286::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
