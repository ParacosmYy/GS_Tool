/**
 * @file algo_7627.cpp
 */
#include "dsp7627/algo_7627.h"
QVector<double> algo_7627::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
