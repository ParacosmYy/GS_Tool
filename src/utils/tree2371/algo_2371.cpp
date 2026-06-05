/**
 * @file algo_2371.cpp
 * @brief Algorithm module 2371
 */
#include "tree2371/algo_2371.h"
QVector<double> algo_2371::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
