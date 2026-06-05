/**
 * @file algo_2348.cpp
 * @brief Algorithm module 2348
 */
#include "fft2348/algo_2348.h"
QVector<double> algo_2348::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
