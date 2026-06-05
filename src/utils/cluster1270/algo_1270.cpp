/**
 * @file algo_1270.cpp
 * @brief Algorithm module 1270
 */
#include "cluster1270/algo_1270.h"
QVector<double> algo_1270::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
