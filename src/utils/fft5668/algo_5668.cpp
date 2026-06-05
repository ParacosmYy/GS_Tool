/**
 * @file algo_5668.cpp
 */
#include "fft5668/algo_5668.h"
QVector<double> algo_5668::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
