/**
 * @file algo_6767.cpp
 */
#include "dsp6767/algo_6767.h"
QVector<double> algo_6767::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
