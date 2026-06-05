/**
 * @file algo_4328.cpp
 */
#include "fft4328/algo_4328.h"
QVector<double> algo_4328::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
