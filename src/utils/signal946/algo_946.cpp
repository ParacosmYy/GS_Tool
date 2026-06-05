/**
 * @file algo_946.cpp
 * @brief Algorithm module 946
 */
#include "signal946/algo_946.h"
QVector<double> algo_946::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
