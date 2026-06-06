/**
 * @file algo_7787.cpp
 */
#include "dsp7787/algo_7787.h"
QVector<double> algo_7787::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
