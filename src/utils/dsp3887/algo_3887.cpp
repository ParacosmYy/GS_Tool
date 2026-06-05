/**
 * @file algo_3887.cpp
 */
#include "dsp3887/algo_3887.h"
QVector<double> algo_3887::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
