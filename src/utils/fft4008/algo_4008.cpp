/**
 * @file algo_4008.cpp
 */
#include "fft4008/algo_4008.h"
QVector<double> algo_4008::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
