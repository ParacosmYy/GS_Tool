/**
 * @file algo_4227.cpp
 */
#include "dsp4227/algo_4227.h"
QVector<double> algo_4227::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
