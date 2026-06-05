/**
 * @file algo_1592.cpp
 * @brief Algorithm module 1592
 */
#include "compress1592/algo_1592.h"
QVector<double> algo_1592::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
