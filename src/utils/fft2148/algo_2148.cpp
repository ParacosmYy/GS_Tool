/**
 * @file algo_2148.cpp
 * @brief Algorithm module 2148
 */
#include "fft2148/algo_2148.h"
QVector<double> algo_2148::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
