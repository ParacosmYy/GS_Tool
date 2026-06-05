/**
 * @file algo_4728.cpp
 */
#include "fft4728/algo_4728.h"
QVector<double> algo_4728::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
