/**
 * @file algo_3607.cpp
 */
#include "dsp3607/algo_3607.h"
QVector<double> algo_3607::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
