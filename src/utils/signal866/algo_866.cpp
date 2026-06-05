/**
 * @file algo_866.cpp
 * @brief Algorithm module 866
 */
#include "signal866/algo_866.h"
QVector<double> algo_866::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
