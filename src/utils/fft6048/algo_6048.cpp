/**
 * @file algo_6048.cpp
 */
#include "fft6048/algo_6048.h"
QVector<double> algo_6048::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
