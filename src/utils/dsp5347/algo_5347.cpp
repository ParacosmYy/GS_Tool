/**
 * @file algo_5347.cpp
 */
#include "dsp5347/algo_5347.h"
QVector<double> algo_5347::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
