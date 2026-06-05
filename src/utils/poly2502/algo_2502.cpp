/**
 * @file algo_2502.cpp
 * @brief Algorithm module 2502
 */
#include "poly2502/algo_2502.h"
QVector<double> algo_2502::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
