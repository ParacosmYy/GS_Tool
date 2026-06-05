/**
 * @file algo_2121.cpp
 * @brief Algorithm module 2121
 */
#include "interp2121/algo_2121.h"
QVector<double> algo_2121::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
