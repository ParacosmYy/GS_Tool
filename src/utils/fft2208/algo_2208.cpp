/**
 * @file algo_2208.cpp
 * @brief Algorithm module 2208
 */
#include "fft2208/algo_2208.h"
QVector<double> algo_2208::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
