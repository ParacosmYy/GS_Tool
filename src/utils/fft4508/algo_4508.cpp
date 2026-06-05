/**
 * @file algo_4508.cpp
 */
#include "fft4508/algo_4508.h"
QVector<double> algo_4508::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
