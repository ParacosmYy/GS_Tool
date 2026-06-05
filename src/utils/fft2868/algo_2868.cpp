/**
 * @file algo_2868.cpp
 */
#include "fft2868/algo_2868.h"
QVector<double> algo_2868::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
