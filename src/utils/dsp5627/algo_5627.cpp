/**
 * @file algo_5627.cpp
 */
#include "dsp5627/algo_5627.h"
QVector<double> algo_5627::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
