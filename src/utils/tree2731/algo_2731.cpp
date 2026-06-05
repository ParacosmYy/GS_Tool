/**
 * @file algo_2731.cpp
 * @brief Algorithm module 2731
 */
#include "tree2731/algo_2731.h"
QVector<double> algo_2731::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
