/**
 * @file algo_3847.cpp
 */
#include "dsp3847/algo_3847.h"
QVector<double> algo_3847::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
