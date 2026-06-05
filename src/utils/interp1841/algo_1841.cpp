/**
 * @file algo_1841.cpp
 * @brief Algorithm module 1841
 */
#include "interp1841/algo_1841.h"
QVector<double> algo_1841::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
