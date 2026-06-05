/**
 * @file algo_2292.cpp
 * @brief Algorithm module 2292
 */
#include "compress2292/algo_2292.h"
QVector<double> algo_2292::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
