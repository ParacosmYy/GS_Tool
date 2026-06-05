/**
 * @file algo_3768.cpp
 */
#include "fft3768/algo_3768.h"
QVector<double> algo_3768::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
