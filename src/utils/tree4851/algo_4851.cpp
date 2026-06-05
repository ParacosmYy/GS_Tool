/**
 * @file algo_4851.cpp
 */
#include "tree4851/algo_4851.h"
QVector<double> algo_4851::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
