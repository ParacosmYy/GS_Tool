/**
 * @file algo_2548.cpp
 * @brief Algorithm module 2548
 */
#include "fft2548/algo_2548.h"
QVector<double> algo_2548::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
