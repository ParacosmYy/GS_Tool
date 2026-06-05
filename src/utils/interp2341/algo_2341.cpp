/**
 * @file algo_2341.cpp
 * @brief Algorithm module 2341
 */
#include "interp2341/algo_2341.h"
QVector<double> algo_2341::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
