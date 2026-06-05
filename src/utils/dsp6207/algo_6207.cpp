/**
 * @file algo_6207.cpp
 */
#include "dsp6207/algo_6207.h"
QVector<double> algo_6207::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
