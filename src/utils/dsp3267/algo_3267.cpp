/**
 * @file algo_3267.cpp
 */
#include "dsp3267/algo_3267.h"
QVector<double> algo_3267::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
