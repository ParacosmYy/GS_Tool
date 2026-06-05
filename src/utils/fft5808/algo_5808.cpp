/**
 * @file algo_5808.cpp
 */
#include "fft5808/algo_5808.h"
QVector<double> algo_5808::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
