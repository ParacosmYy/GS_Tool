/**
 * @file algo_7208.cpp
 */
#include "fft7208/algo_7208.h"
QVector<double> algo_7208::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
