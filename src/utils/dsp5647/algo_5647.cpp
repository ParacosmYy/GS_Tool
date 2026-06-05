/**
 * @file algo_5647.cpp
 */
#include "dsp5647/algo_5647.h"
QVector<double> algo_5647::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
