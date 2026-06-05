/**
 * @file algo_2571.cpp
 * @brief Algorithm module 2571
 */
#include "tree2571/algo_2571.h"
QVector<double> algo_2571::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
