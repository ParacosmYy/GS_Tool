/**
 * @file algo_1941.cpp
 * @brief Algorithm module 1941
 */
#include "interp1941/algo_1941.h"
QVector<double> algo_1941::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
