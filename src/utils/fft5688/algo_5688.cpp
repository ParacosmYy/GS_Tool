/**
 * @file algo_5688.cpp
 */
#include "fft5688/algo_5688.h"
QVector<double> algo_5688::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
