/**
 * @file algo_6428.cpp
 */
#include "fft6428/algo_6428.h"
QVector<double> algo_6428::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
