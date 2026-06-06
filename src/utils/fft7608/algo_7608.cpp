/**
 * @file algo_7608.cpp
 */
#include "fft7608/algo_7608.h"
QVector<double> algo_7608::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
