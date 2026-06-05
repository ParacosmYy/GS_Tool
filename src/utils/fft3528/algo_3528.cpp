/**
 * @file algo_3528.cpp
 */
#include "fft3528/algo_3528.h"
QVector<double> algo_3528::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
