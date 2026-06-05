/**
 * @file algo_6142.cpp
 */
#include "poly6142/algo_6142.h"
QVector<double> algo_6142::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
