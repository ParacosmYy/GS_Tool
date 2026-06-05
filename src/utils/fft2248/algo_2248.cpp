/**
 * @file algo_2248.cpp
 * @brief Algorithm module 2248
 */
#include "fft2248/algo_2248.h"
QVector<double> algo_2248::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
