/**
 * @file algo_2108.cpp
 * @brief Algorithm module 2108
 */
#include "fft2108/algo_2108.h"
QVector<double> algo_2108::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
