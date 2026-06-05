/**
 * @file algo_1752.cpp
 * @brief Algorithm module 1752
 */
#include "compress1752/algo_1752.h"
QVector<double> algo_1752::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
