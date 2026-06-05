/**
 * @file algo_2226.cpp
 * @brief Algorithm module 2226
 */
#include "signal2226/algo_2226.h"
QVector<double> algo_2226::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
