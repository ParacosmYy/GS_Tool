/**
 * @file algo_6427.cpp
 */
#include "dsp6427/algo_6427.h"
QVector<double> algo_6427::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
