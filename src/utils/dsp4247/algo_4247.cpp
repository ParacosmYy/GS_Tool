/**
 * @file algo_4247.cpp
 */
#include "dsp4247/algo_4247.h"
QVector<double> algo_4247::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
