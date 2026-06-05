/**
 * @file algo_6227.cpp
 */
#include "dsp6227/algo_6227.h"
QVector<double> algo_6227::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
