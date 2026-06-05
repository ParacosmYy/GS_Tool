/**
 * @file algo_5408.cpp
 */
#include "fft5408/algo_5408.h"
QVector<double> algo_5408::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
