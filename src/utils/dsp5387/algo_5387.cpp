/**
 * @file algo_5387.cpp
 */
#include "dsp5387/algo_5387.h"
QVector<double> algo_5387::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
