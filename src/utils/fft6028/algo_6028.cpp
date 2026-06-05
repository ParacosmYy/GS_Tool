/**
 * @file algo_6028.cpp
 */
#include "fft6028/algo_6028.h"
QVector<double> algo_6028::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
