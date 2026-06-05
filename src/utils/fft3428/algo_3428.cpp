/**
 * @file algo_3428.cpp
 */
#include "fft3428/algo_3428.h"
QVector<double> algo_3428::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
