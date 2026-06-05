/**
 * @file algo_5867.cpp
 */
#include "dsp5867/algo_5867.h"
QVector<double> algo_5867::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
