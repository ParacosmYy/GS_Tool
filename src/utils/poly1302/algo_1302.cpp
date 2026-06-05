/**
 * @file algo_1302.cpp
 * @brief Algorithm module 1302
 */
#include "poly1302/algo_1302.h"
QVector<double> algo_1302::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
