/**
 * @file algo_1108.cpp
 * @brief Algorithm module 1108
 */
#include "fft1108/algo_1108.h"
QVector<double> algo_1108::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
