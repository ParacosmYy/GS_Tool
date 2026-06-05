/**
 * @file algo_2441.cpp
 * @brief Algorithm module 2441
 */
#include "interp2441/algo_2441.h"
QVector<double> algo_2441::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
