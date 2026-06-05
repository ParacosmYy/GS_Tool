/**
 * @file algo_3988.cpp
 */
#include "fft3988/algo_3988.h"
QVector<double> algo_3988::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
