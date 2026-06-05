/**
 * @file algo_6828.cpp
 */
#include "fft6828/algo_6828.h"
QVector<double> algo_6828::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
