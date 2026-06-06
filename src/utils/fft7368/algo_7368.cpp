/**
 * @file algo_7368.cpp
 */
#include "fft7368/algo_7368.h"
QVector<double> algo_7368::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
