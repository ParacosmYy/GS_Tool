/**
 * @file algo_3728.cpp
 */
#include "fft3728/algo_3728.h"
QVector<double> algo_3728::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
