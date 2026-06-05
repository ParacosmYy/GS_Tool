/**
 * @file algo_6468.cpp
 */
#include "fft6468/algo_6468.h"
QVector<double> algo_6468::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
