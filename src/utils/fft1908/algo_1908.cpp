/**
 * @file algo_1908.cpp
 * @brief Algorithm module 1908
 */
#include "fft1908/algo_1908.h"
QVector<double> algo_1908::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
