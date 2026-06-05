/**
 * @file algo_1494.cpp
 * @brief Algorithm module 1494
 */
#include "numeric1494/algo_1494.h"
QVector<double> algo_1494::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
