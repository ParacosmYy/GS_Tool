/**
 * @file algo_2748.cpp
 * @brief Algorithm module 2748
 */
#include "fft2748/algo_2748.h"
QVector<double> algo_2748::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
