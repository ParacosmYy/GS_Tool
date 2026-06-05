/**
 * @file algo_2328.cpp
 * @brief Algorithm module 2328
 */
#include "fft2328/algo_2328.h"
QVector<double> algo_2328::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
