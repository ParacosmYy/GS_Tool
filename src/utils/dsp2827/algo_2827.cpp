/**
 * @file algo_2827.cpp
 */
#include "dsp2827/algo_2827.h"
QVector<double> algo_2827::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
