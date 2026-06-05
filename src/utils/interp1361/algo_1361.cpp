/**
 * @file algo_1361.cpp
 * @brief Algorithm module 1361
 */
#include "interp1361/algo_1361.h"
QVector<double> algo_1361::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
