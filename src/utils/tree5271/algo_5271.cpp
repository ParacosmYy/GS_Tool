/**
 * @file algo_5271.cpp
 */
#include "tree5271/algo_5271.h"
QVector<double> algo_5271::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
