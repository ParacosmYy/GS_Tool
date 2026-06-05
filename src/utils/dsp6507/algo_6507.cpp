/**
 * @file algo_6507.cpp
 */
#include "dsp6507/algo_6507.h"
QVector<double> algo_6507::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
