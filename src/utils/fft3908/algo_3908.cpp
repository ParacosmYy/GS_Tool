/**
 * @file algo_3908.cpp
 */
#include "fft3908/algo_3908.h"
QVector<double> algo_3908::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
