/**
 * @file algo_5622.cpp
 */
#include "poly5622/algo_5622.h"
QVector<double> algo_5622::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
