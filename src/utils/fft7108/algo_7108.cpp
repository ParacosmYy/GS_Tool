/**
 * @file algo_7108.cpp
 */
#include "fft7108/algo_7108.h"
QVector<double> algo_7108::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
