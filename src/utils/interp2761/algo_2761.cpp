/**
 * @file algo_2761.cpp
 * @brief Algorithm module 2761
 */
#include "interp2761/algo_2761.h"
QVector<double> algo_2761::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
