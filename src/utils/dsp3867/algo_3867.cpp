/**
 * @file algo_3867.cpp
 */
#include "dsp3867/algo_3867.h"
QVector<double> algo_3867::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
