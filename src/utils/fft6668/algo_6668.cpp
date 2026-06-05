/**
 * @file algo_6668.cpp
 */
#include "fft6668/algo_6668.h"
QVector<double> algo_6668::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
