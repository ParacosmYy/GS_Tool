/**
 * @file algo_2591.cpp
 * @brief Algorithm module 2591
 */
#include "tree2591/algo_2591.h"
QVector<double> algo_2591::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
