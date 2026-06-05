/**
 * @file algo_3008.cpp
 */
#include "fft3008/algo_3008.h"
QVector<double> algo_3008::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
