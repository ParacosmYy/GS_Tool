/**
 * @file algo_1641.cpp
 * @brief Algorithm module 1641
 */
#include "interp1641/algo_1641.h"
QVector<double> algo_1641::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
