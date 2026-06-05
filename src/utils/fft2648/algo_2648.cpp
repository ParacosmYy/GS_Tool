/**
 * @file algo_2648.cpp
 * @brief Algorithm module 2648
 */
#include "fft2648/algo_2648.h"
QVector<double> algo_2648::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
