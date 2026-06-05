/**
 * @file algo_3668.cpp
 */
#include "fft3668/algo_3668.h"
QVector<double> algo_3668::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
