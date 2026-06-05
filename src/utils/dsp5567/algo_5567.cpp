/**
 * @file algo_5567.cpp
 */
#include "dsp5567/algo_5567.h"
QVector<double> algo_5567::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
