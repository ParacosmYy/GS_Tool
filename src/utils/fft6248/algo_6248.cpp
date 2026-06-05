/**
 * @file algo_6248.cpp
 */
#include "fft6248/algo_6248.h"
QVector<double> algo_6248::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
