/**
 * @file algo_7228.cpp
 */
#include "fft7228/algo_7228.h"
QVector<double> algo_7228::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
