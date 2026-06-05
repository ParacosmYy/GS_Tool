/**
 * @file algo_1808.cpp
 * @brief Algorithm module 1808
 */
#include "fft1808/algo_1808.h"
QVector<double> algo_1808::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
