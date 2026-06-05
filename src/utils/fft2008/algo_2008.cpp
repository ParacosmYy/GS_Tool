/**
 * @file algo_2008.cpp
 * @brief Algorithm module 2008
 */
#include "fft2008/algo_2008.h"
QVector<double> algo_2008::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
