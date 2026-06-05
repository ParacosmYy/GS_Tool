/**
 * @file algo_5468.cpp
 */
#include "fft5468/algo_5468.h"
QVector<double> algo_5468::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
