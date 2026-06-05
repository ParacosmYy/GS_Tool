/**
 * @file algo_5487.cpp
 */
#include "dsp5487/algo_5487.h"
QVector<double> algo_5487::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
