/**
 * @file algo_1891.cpp
 * @brief Algorithm module 1891
 */
#include "tree1891/algo_1891.h"
QVector<double> algo_1891::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
