/**
 * @file algo_2847.cpp
 */
#include "dsp2847/algo_2847.h"
QVector<double> algo_2847::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
