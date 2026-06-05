/**
 * @file algo_3567.cpp
 */
#include "dsp3567/algo_3567.h"
QVector<double> algo_3567::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
