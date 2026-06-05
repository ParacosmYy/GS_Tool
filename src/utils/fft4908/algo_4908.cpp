/**
 * @file algo_4908.cpp
 */
#include "fft4908/algo_4908.h"
QVector<double> algo_4908::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
