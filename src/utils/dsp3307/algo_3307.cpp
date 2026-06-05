/**
 * @file algo_3307.cpp
 */
#include "dsp3307/algo_3307.h"
QVector<double> algo_3307::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
