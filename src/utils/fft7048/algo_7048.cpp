/**
 * @file algo_7048.cpp
 */
#include "fft7048/algo_7048.h"
QVector<double> algo_7048::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
