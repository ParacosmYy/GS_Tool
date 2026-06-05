/**
 * @file algo_2048.cpp
 * @brief Algorithm module 2048
 */
#include "fft2048/algo_2048.h"
QVector<double> algo_2048::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
