/**
 * @file algo_1502.cpp
 * @brief Algorithm module 1502
 */
#include "poly1502/algo_1502.h"
QVector<double> algo_1502::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
