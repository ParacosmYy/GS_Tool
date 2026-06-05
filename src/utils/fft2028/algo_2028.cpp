/**
 * @file algo_2028.cpp
 * @brief Algorithm module 2028
 */
#include "fft2028/algo_2028.h"
QVector<double> algo_2028::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
