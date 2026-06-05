/**
 * @file algo_1282.cpp
 * @brief Algorithm module 1282
 */
#include "poly1282/algo_1282.h"
QVector<double> algo_1282::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
