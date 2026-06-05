/**
 * @file algo_3448.cpp
 */
#include "fft3448/algo_3448.h"
QVector<double> algo_3448::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
