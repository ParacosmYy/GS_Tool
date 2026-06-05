/**
 * @file algo_2141.cpp
 * @brief Algorithm module 2141
 */
#include "interp2141/algo_2141.h"
QVector<double> algo_2141::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
