/**
 * @file algo_3268.cpp
 */
#include "fft3268/algo_3268.h"
QVector<double> algo_3268::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
