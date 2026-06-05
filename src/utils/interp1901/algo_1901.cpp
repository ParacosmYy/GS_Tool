/**
 * @file algo_1901.cpp
 * @brief Algorithm module 1901
 */
#include "interp1901/algo_1901.h"
QVector<double> algo_1901::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
