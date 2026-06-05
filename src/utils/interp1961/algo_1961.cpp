/**
 * @file algo_1961.cpp
 * @brief Algorithm module 1961
 */
#include "interp1961/algo_1961.h"
QVector<double> algo_1961::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
