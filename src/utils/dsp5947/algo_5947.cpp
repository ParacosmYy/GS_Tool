/**
 * @file algo_5947.cpp
 */
#include "dsp5947/algo_5947.h"
QVector<double> algo_5947::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
