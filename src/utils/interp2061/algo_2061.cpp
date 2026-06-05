/**
 * @file algo_2061.cpp
 * @brief Algorithm module 2061
 */
#include "interp2061/algo_2061.h"
QVector<double> algo_2061::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
