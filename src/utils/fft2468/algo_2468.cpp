/**
 * @file algo_2468.cpp
 * @brief Algorithm module 2468
 */
#include "fft2468/algo_2468.h"
QVector<double> algo_2468::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
