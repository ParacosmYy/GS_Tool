/**
 * @file algo_5728.cpp
 */
#include "fft5728/algo_5728.h"
QVector<double> algo_5728::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
