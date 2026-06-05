/**
 * @file algo_4708.cpp
 */
#include "fft4708/algo_4708.h"
QVector<double> algo_4708::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
