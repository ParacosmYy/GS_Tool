/**
 * @file algo_6887.cpp
 */
#include "dsp6887/algo_6887.h"
QVector<double> algo_6887::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
