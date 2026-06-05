/**
 * @file algo_6168.cpp
 */
#include "fft6168/algo_6168.h"
QVector<double> algo_6168::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
