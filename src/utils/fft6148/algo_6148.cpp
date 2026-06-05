/**
 * @file algo_6148.cpp
 */
#include "fft6148/algo_6148.h"
QVector<double> algo_6148::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
