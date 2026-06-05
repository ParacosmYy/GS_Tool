/**
 * @file algo_6587.cpp
 */
#include "dsp6587/algo_6587.h"
QVector<double> algo_6587::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
