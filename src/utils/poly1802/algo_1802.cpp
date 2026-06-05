/**
 * @file algo_1802.cpp
 * @brief Algorithm module 1802
 */
#include "poly1802/algo_1802.h"
QVector<double> algo_1802::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
