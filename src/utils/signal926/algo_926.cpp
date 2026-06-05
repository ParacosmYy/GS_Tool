/**
 * @file algo_926.cpp
 * @brief Algorithm module 926
 */
#include "signal926/algo_926.h"
QVector<double> algo_926::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
