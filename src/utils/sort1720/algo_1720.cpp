/**
 * @file algo_1720.cpp
 * @brief Algorithm module 1720
 */
#include "sort1720/algo_1720.h"
QVector<double> algo_1720::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
