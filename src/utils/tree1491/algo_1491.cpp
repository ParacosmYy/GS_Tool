/**
 * @file algo_1491.cpp
 * @brief Algorithm module 1491
 */
#include "tree1491/algo_1491.h"
QVector<double> algo_1491::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
