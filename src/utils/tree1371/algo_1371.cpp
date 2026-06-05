/**
 * @file algo_1371.cpp
 * @brief Algorithm module 1371
 */
#include "tree1371/algo_1371.h"
QVector<double> algo_1371::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
