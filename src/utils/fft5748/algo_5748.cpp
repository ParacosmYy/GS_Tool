/**
 * @file algo_5748.cpp
 */
#include "fft5748/algo_5748.h"
QVector<double> algo_5748::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
