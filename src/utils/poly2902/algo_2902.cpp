/**
 * @file algo_2902.cpp
 */
#include "poly2902/algo_2902.h"
QVector<double> algo_2902::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
