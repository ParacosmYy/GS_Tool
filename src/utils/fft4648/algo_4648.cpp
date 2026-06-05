/**
 * @file algo_4648.cpp
 */
#include "fft4648/algo_4648.h"
QVector<double> algo_4648::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
