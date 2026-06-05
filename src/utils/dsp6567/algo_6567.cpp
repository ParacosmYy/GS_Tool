/**
 * @file algo_6567.cpp
 */
#include "dsp6567/algo_6567.h"
QVector<double> algo_6567::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
