/**
 * @file algo_5008.cpp
 */
#include "fft5008/algo_5008.h"
QVector<double> algo_5008::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
