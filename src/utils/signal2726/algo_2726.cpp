/**
 * @file algo_2726.cpp
 * @brief Algorithm module 2726
 */
#include "signal2726/algo_2726.h"
QVector<double> algo_2726::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
