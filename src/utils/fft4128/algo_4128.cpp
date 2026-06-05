/**
 * @file algo_4128.cpp
 */
#include "fft4128/algo_4128.h"
QVector<double> algo_4128::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
