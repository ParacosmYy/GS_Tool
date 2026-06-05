/**
 * @file algo_3348.cpp
 */
#include "fft3348/algo_3348.h"
QVector<double> algo_3348::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
