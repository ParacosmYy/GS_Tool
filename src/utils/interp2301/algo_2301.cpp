/**
 * @file algo_2301.cpp
 * @brief Algorithm module 2301
 */
#include "interp2301/algo_2301.h"
QVector<double> algo_2301::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
