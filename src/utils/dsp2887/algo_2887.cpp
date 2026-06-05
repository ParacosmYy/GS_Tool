/**
 * @file algo_2887.cpp
 */
#include "dsp2887/algo_2887.h"
QVector<double> algo_2887::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
