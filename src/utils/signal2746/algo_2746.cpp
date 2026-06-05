/**
 * @file algo_2746.cpp
 * @brief Algorithm module 2746
 */
#include "signal2746/algo_2746.h"
QVector<double> algo_2746::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
