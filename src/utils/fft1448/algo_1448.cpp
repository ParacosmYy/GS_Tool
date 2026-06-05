/**
 * @file algo_1448.cpp
 * @brief Algorithm module 1448
 */
#include "fft1448/algo_1448.h"
QVector<double> algo_1448::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
