/**
 * @file algo_6868.cpp
 */
#include "fft6868/algo_6868.h"
QVector<double> algo_6868::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
