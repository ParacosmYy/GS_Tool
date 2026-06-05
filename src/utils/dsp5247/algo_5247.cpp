/**
 * @file algo_5247.cpp
 */
#include "dsp5247/algo_5247.h"
QVector<double> algo_5247::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
