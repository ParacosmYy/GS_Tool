/**
 * @file algo_2446.cpp
 * @brief Algorithm module 2446
 */
#include "signal2446/algo_2446.h"
QVector<double> algo_2446::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
