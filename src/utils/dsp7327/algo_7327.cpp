/**
 * @file algo_7327.cpp
 */
#include "dsp7327/algo_7327.h"
QVector<double> algo_7327::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
