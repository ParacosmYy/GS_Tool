/**
 * @file algo_5067.cpp
 */
#include "dsp5067/algo_5067.h"
QVector<double> algo_5067::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
