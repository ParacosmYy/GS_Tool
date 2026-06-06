/**
 * @file algo_7287.cpp
 */
#include "dsp7287/algo_7287.h"
QVector<double> algo_7287::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
