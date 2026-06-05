/**
 * @file algo_4847.cpp
 */
#include "dsp4847/algo_4847.h"
QVector<double> algo_4847::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
