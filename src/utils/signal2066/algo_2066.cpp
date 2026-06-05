/**
 * @file algo_2066.cpp
 * @brief Algorithm module 2066
 */
#include "signal2066/algo_2066.h"
QVector<double> algo_2066::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
