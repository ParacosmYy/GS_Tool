/**
 * @file algo_2308.cpp
 * @brief Algorithm module 2308
 */
#include "fft2308/algo_2308.h"
QVector<double> algo_2308::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
