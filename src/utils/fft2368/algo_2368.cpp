/**
 * @file algo_2368.cpp
 * @brief Algorithm module 2368
 */
#include "fft2368/algo_2368.h"
QVector<double> algo_2368::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
