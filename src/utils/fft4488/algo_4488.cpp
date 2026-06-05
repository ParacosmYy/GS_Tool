/**
 * @file algo_4488.cpp
 */
#include "fft4488/algo_4488.h"
QVector<double> algo_4488::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
