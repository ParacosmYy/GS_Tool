/**
 * @file algo_7267.cpp
 */
#include "dsp7267/algo_7267.h"
QVector<double> algo_7267::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
