/**
 * @file algo_4148.cpp
 */
#include "fft4148/algo_4148.h"
QVector<double> algo_4148::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
