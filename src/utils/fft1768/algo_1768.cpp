/**
 * @file algo_1768.cpp
 * @brief Algorithm module 1768
 */
#include "fft1768/algo_1768.h"
QVector<double> algo_1768::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
