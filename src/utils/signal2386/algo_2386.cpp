/**
 * @file algo_2386.cpp
 * @brief Algorithm module 2386
 */
#include "signal2386/algo_2386.h"
QVector<double> algo_2386::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
