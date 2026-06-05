/**
 * @file algo_1571.cpp
 * @brief Algorithm module 1571
 */
#include "tree1571/algo_1571.h"
QVector<double> algo_1571::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
