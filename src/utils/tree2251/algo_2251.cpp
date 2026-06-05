/**
 * @file algo_2251.cpp
 * @brief Algorithm module 2251
 */
#include "tree2251/algo_2251.h"
QVector<double> algo_2251::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
