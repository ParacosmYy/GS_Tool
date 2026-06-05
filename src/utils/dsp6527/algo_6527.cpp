/**
 * @file algo_6527.cpp
 */
#include "dsp6527/algo_6527.h"
QVector<double> algo_6527::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
