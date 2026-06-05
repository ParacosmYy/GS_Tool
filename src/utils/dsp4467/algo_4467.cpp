/**
 * @file algo_4467.cpp
 */
#include "dsp4467/algo_4467.h"
QVector<double> algo_4467::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
