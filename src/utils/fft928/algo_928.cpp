/**
 * @file algo_928.cpp
 * @brief Algorithm module 928
 */
#include "fft928/algo_928.h"
QVector<double> algo_928::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
