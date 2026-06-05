/**
 * @file algo_1101.cpp
 * @brief Algorithm module 1101
 */
#include "interp1101/algo_1101.h"
QVector<double> algo_1101::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
