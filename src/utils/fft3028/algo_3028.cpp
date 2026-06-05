/**
 * @file algo_3028.cpp
 */
#include "fft3028/algo_3028.h"
QVector<double> algo_3028::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
