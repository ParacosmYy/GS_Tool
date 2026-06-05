/**
 * @file algo_2641.cpp
 * @brief Algorithm module 2641
 */
#include "interp2641/algo_2641.h"
QVector<double> algo_2641::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
