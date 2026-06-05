/**
 * @file algo_5687.cpp
 */
#include "dsp5687/algo_5687.h"
QVector<double> algo_5687::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
