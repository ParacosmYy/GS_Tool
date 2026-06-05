/**
 * @file algo_1928.cpp
 * @brief Algorithm module 1928
 */
#include "fft1928/algo_1928.h"
QVector<double> algo_1928::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
