/**
 * @file algo_2188.cpp
 * @brief Algorithm module 2188
 */
#include "fft2188/algo_2188.h"
QVector<double> algo_2188::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
