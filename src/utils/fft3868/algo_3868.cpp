/**
 * @file algo_3868.cpp
 */
#include "fft3868/algo_3868.h"
QVector<double> algo_3868::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
