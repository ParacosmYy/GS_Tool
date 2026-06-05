/**
 * @file algo_5708.cpp
 */
#include "fft5708/algo_5708.h"
QVector<double> algo_5708::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
