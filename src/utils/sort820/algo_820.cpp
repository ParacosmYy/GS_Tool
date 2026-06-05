/**
 * @file algo_820.cpp
 * @brief Algorithm module 820
 */
#include "sort820/algo_820.h"
QVector<double> algo_820::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
