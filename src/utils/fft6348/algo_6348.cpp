/**
 * @file algo_6348.cpp
 */
#include "fft6348/algo_6348.h"
QVector<double> algo_6348::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
