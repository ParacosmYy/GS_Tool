/**
 * @file algo_2708.cpp
 * @brief Algorithm module 2708
 */
#include "fft2708/algo_2708.h"
QVector<double> algo_2708::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
