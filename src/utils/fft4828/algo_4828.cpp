/**
 * @file algo_4828.cpp
 */
#include "fft4828/algo_4828.h"
QVector<double> algo_4828::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
