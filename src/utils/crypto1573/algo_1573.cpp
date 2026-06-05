/**
 * @file algo_1573.cpp
 * @brief Algorithm module 1573
 */
#include "crypto1573/algo_1573.h"
QVector<double> algo_1573::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
