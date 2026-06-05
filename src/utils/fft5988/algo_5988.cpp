/**
 * @file algo_5988.cpp
 */
#include "fft5988/algo_5988.h"
QVector<double> algo_5988::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
