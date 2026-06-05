/**
 * @file algo_4667.cpp
 */
#include "dsp4667/algo_4667.h"
QVector<double> algo_4667::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
