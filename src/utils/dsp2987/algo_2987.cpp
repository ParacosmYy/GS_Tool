/**
 * @file algo_2987.cpp
 */
#include "dsp2987/algo_2987.h"
QVector<double> algo_2987::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
