/**
 * @file algo_1840.cpp
 * @brief Algorithm module 1840
 */
#include "sort1840/algo_1840.h"
QVector<double> algo_1840::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
