/**
 * @file algo_5162.cpp
 */
#include "poly5162/algo_5162.h"
QVector<double> algo_5162::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
