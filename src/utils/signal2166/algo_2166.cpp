/**
 * @file algo_2166.cpp
 * @brief Algorithm module 2166
 */
#include "signal2166/algo_2166.h"
QVector<double> algo_2166::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
