/**
 * @file algo_1620.cpp
 * @brief Algorithm module 1620
 */
#include "sort1620/algo_1620.h"
QVector<double> algo_1620::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
