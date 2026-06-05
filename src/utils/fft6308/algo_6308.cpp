/**
 * @file algo_6308.cpp
 */
#include "fft6308/algo_6308.h"
QVector<double> algo_6308::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
