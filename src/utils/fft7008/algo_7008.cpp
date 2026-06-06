/**
 * @file algo_7008.cpp
 */
#include "fft7008/algo_7008.h"
QVector<double> algo_7008::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
