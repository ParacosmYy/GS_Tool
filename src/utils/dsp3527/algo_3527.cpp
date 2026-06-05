/**
 * @file algo_3527.cpp
 */
#include "dsp3527/algo_3527.h"
QVector<double> algo_3527::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
