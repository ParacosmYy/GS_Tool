/**
 * @file algo_2628.cpp
 * @brief Algorithm module 2628
 */
#include "fft2628/algo_2628.h"
QVector<double> algo_2628::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
