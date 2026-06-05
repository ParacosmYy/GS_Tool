/**
 * @file algo_6088.cpp
 */
#include "fft6088/algo_6088.h"
QVector<double> algo_6088::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
