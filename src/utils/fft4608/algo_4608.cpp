/**
 * @file algo_4608.cpp
 */
#include "fft4608/algo_4608.h"
QVector<double> algo_4608::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
