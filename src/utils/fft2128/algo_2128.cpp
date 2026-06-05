/**
 * @file algo_2128.cpp
 * @brief Algorithm module 2128
 */
#include "fft2128/algo_2128.h"
QVector<double> algo_2128::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
