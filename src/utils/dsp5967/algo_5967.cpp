/**
 * @file algo_5967.cpp
 */
#include "dsp5967/algo_5967.h"
QVector<double> algo_5967::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
