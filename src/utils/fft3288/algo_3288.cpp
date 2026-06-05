/**
 * @file algo_3288.cpp
 */
#include "fft3288/algo_3288.h"
QVector<double> algo_3288::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
