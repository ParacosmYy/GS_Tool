/**
 * @file algo_3787.cpp
 */
#include "dsp3787/algo_3787.h"
QVector<double> algo_3787::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
