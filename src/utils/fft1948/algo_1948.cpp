/**
 * @file algo_1948.cpp
 * @brief Algorithm module 1948
 */
#include "fft1948/algo_1948.h"
QVector<double> algo_1948::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
