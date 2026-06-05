/**
 * @file algo_2471.cpp
 * @brief Algorithm module 2471
 */
#include "tree2471/algo_2471.h"
QVector<double> algo_2471::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
