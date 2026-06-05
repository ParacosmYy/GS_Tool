/**
 * @file algo_3647.cpp
 */
#include "dsp3647/algo_3647.h"
QVector<double> algo_3647::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
