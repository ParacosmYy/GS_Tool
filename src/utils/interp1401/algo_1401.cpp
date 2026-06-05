/**
 * @file algo_1401.cpp
 * @brief Algorithm module 1401
 */
#include "interp1401/algo_1401.h"
QVector<double> algo_1401::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
