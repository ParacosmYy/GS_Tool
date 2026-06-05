/**
 * @file algo_2448.cpp
 * @brief Algorithm module 2448
 */
#include "fft2448/algo_2448.h"
QVector<double> algo_2448::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
