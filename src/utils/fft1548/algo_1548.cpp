/**
 * @file algo_1548.cpp
 * @brief Algorithm module 1548
 */
#include "fft1548/algo_1548.h"
QVector<double> algo_1548::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
