/**
 * @file algo_6968.cpp
 */
#include "fft6968/algo_6968.h"
QVector<double> algo_6968::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
