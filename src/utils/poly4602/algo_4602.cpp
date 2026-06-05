/**
 * @file algo_4602.cpp
 */
#include "poly4602/algo_4602.h"
QVector<double> algo_4602::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
