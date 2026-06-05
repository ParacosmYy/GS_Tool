/**
 * @file algo_3007.cpp
 */
#include "dsp3007/algo_3007.h"
QVector<double> algo_3007::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
