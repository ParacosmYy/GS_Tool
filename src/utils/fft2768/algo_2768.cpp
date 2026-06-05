/**
 * @file algo_2768.cpp
 * @brief Algorithm module 2768
 */
#include "fft2768/algo_2768.h"
QVector<double> algo_2768::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
