/**
 * @file algo_1166.cpp
 * @brief Algorithm module 1166
 */
#include "signal1166/algo_1166.h"
QVector<double> algo_1166::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
