/**
 * @file algo_1260.cpp
 * @brief Algorithm module 1260
 */
#include "sort1260/algo_1260.h"
QVector<double> algo_1260::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
