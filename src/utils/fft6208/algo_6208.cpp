/**
 * @file algo_6208.cpp
 */
#include "fft6208/algo_6208.h"
QVector<double> algo_6208::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
