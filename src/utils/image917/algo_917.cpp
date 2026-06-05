/**
 * @file algo_917.cpp
 * @brief Algorithm module 917
 */
#include "image917/algo_917.h"
QVector<double> algo_917::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
