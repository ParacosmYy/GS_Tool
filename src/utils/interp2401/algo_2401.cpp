/**
 * @file algo_2401.cpp
 * @brief Algorithm module 2401
 */
#include "interp2401/algo_2401.h"
QVector<double> algo_2401::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
