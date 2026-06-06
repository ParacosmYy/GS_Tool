/**
 * @file algo_7768.cpp
 */
#include "fft7768/algo_7768.h"
QVector<double> algo_7768::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
