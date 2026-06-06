/**
 * @file algo_6928.cpp
 */
#include "fft6928/algo_6928.h"
QVector<double> algo_6928::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
