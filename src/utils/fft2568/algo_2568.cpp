/**
 * @file algo_2568.cpp
 * @brief Algorithm module 2568
 */
#include "fft2568/algo_2568.h"
QVector<double> algo_2568::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
