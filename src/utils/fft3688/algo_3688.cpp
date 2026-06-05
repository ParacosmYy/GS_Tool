/**
 * @file algo_3688.cpp
 */
#include "fft3688/algo_3688.h"
QVector<double> algo_3688::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
