/**
 * @file algo_1091.cpp
 * @brief Algorithm module 1091
 */
#include "tree1091/algo_1091.h"
QVector<double> algo_1091::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
