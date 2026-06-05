/**
 * @file algo_6307.cpp
 */
#include "dsp6307/algo_6307.h"
QVector<double> algo_6307::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
