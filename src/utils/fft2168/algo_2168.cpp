/**
 * @file algo_2168.cpp
 * @brief Algorithm module 2168
 */
#include "fft2168/algo_2168.h"
QVector<double> algo_2168::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
