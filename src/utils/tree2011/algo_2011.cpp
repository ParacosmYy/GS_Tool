/**
 * @file algo_2011.cpp
 * @brief Algorithm module 2011
 */
#include "tree2011/algo_2011.h"
QVector<double> algo_2011::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
