/**
 * @file algo_1668.cpp
 * @brief Algorithm module 1668
 */
#include "fft1668/algo_1668.h"
QVector<double> algo_1668::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
