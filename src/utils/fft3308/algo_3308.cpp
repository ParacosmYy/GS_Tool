/**
 * @file algo_3308.cpp
 */
#include "fft3308/algo_3308.h"
QVector<double> algo_3308::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
