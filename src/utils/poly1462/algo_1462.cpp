/**
 * @file algo_1462.cpp
 * @brief Algorithm module 1462
 */
#include "poly1462/algo_1462.h"
QVector<double> algo_1462::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
