/**
 * @file algo_1201.cpp
 * @brief Algorithm module 1201
 */
#include "interp1201/algo_1201.h"
QVector<double> algo_1201::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
