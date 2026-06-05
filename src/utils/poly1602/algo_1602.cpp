/**
 * @file algo_1602.cpp
 * @brief Algorithm module 1602
 */
#include "poly1602/algo_1602.h"
QVector<double> algo_1602::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
