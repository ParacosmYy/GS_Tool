/**
 * @file algo_4302.cpp
 */
#include "poly4302/algo_4302.h"
QVector<double> algo_4302::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
