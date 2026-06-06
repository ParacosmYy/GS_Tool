/**
 * @file algo_7128.cpp
 */
#include "fft7128/algo_7128.h"
QVector<double> algo_7128::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
