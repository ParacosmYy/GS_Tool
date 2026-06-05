/**
 * @file algo_1248.cpp
 * @brief Algorithm module 1248
 */
#include "fft1248/algo_1248.h"
QVector<double> algo_1248::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
