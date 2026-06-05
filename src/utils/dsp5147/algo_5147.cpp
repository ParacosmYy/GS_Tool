/**
 * @file algo_5147.cpp
 */
#include "dsp5147/algo_5147.h"
QVector<double> algo_5147::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
