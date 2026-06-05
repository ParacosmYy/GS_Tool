/**
 * @file algo_2331.cpp
 * @brief Algorithm module 2331
 */
#include "tree2331/algo_2331.h"
QVector<double> algo_2331::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
