/**
 * @file algo_1160.cpp
 * @brief Algorithm module 1160
 */
#include "sort1160/algo_1160.h"
QVector<double> algo_1160::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
