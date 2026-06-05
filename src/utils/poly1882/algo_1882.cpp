/**
 * @file algo_1882.cpp
 * @brief Algorithm module 1882
 */
#include "poly1882/algo_1882.h"
QVector<double> algo_1882::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
