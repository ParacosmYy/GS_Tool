/**
 * @file algo_4028.cpp
 */
#include "fft4028/algo_4028.h"
QVector<double> algo_4028::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
