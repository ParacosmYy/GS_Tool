/**
 * @file algo_2088.cpp
 * @brief Algorithm module 2088
 */
#include "fft2088/algo_2088.h"
QVector<double> algo_2088::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
