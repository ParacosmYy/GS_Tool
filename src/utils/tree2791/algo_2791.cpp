/**
 * @file algo_2791.cpp
 * @brief Algorithm module 2791
 */
#include "tree2791/algo_2791.h"
QVector<double> algo_2791::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
