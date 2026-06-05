/**
 * @file algo_1820.cpp
 * @brief Algorithm module 1820
 */
#include "sort1820/algo_1820.h"
QVector<double> algo_1820::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
