/**
 * @file algo_4607.cpp
 */
#include "dsp4607/algo_4607.h"
QVector<double> algo_4607::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
