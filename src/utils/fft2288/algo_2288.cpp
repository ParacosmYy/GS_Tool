/**
 * @file algo_2288.cpp
 * @brief Algorithm module 2288
 */
#include "fft2288/algo_2288.h"
QVector<double> algo_2288::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
