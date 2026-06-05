/**
 * @file algo_1420.cpp
 * @brief Algorithm module 1420
 */
#include "sort1420/algo_1420.h"
QVector<double> algo_1420::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
