/**
 * @file algo_1368.cpp
 * @brief Algorithm module 1368
 */
#include "fft1368/algo_1368.h"
QVector<double> algo_1368::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
