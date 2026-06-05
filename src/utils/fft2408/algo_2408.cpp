/**
 * @file algo_2408.cpp
 * @brief Algorithm module 2408
 */
#include "fft2408/algo_2408.h"
QVector<double> algo_2408::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
