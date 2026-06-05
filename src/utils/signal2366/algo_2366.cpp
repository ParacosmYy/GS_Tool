/**
 * @file algo_2366.cpp
 * @brief Algorithm module 2366
 */
#include "signal2366/algo_2366.h"
QVector<double> algo_2366::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
