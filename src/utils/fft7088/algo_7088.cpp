/**
 * @file algo_7088.cpp
 */
#include "fft7088/algo_7088.h"
QVector<double> algo_7088::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
