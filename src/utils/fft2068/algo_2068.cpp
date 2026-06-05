/**
 * @file algo_2068.cpp
 * @brief Algorithm module 2068
 */
#include "fft2068/algo_2068.h"
QVector<double> algo_2068::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
