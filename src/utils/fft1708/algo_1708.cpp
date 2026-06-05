/**
 * @file algo_1708.cpp
 * @brief Algorithm module 1708
 */
#include "fft1708/algo_1708.h"
QVector<double> algo_1708::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
