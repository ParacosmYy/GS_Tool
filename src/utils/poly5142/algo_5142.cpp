/**
 * @file algo_5142.cpp
 */
#include "poly5142/algo_5142.h"
QVector<double> algo_5142::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
