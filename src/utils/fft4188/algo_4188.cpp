/**
 * @file algo_4188.cpp
 */
#include "fft4188/algo_4188.h"
QVector<double> algo_4188::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
