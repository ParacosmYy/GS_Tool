/**
 * @file algo_7428.cpp
 */
#include "fft7428/algo_7428.h"
QVector<double> algo_7428::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
