/**
 * @file algo_7408.cpp
 */
#include "fft7408/algo_7408.h"
QVector<double> algo_7408::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
