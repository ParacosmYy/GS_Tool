/**
 * @file algo_1868.cpp
 * @brief Algorithm module 1868
 */
#include "fft1868/algo_1868.h"
QVector<double> algo_1868::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
