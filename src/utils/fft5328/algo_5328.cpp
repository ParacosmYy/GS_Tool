/**
 * @file algo_5328.cpp
 */
#include "fft5328/algo_5328.h"
QVector<double> algo_5328::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
