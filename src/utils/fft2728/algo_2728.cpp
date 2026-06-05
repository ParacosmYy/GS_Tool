/**
 * @file algo_2728.cpp
 * @brief Algorithm module 2728
 */
#include "fft2728/algo_2728.h"
QVector<double> algo_2728::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
