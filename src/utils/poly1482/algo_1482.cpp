/**
 * @file algo_1482.cpp
 * @brief Algorithm module 1482
 */
#include "poly1482/algo_1482.h"
QVector<double> algo_1482::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
