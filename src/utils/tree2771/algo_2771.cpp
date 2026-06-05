/**
 * @file algo_2771.cpp
 * @brief Algorithm module 2771
 */
#include "tree2771/algo_2771.h"
QVector<double> algo_2771::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
