/**
 * @file algo_2922.cpp
 */
#include "poly2922/algo_2922.h"
QVector<double> algo_2922::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
