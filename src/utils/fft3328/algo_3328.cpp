/**
 * @file algo_3328.cpp
 */
#include "fft3328/algo_3328.h"
QVector<double> algo_3328::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
