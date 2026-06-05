/**
 * @file algo_5547.cpp
 */
#include "dsp5547/algo_5547.h"
QVector<double> algo_5547::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
