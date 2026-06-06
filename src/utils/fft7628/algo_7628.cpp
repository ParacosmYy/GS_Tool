/**
 * @file algo_7628.cpp
 */
#include "fft7628/algo_7628.h"
QVector<double> algo_7628::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
