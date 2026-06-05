/**
 * @file algo_4542.cpp
 */
#include "poly4542/algo_4542.h"
QVector<double> algo_4542::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
