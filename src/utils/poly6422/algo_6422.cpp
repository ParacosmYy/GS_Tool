/**
 * @file algo_6422.cpp
 */
#include "poly6422/algo_6422.h"
QVector<double> algo_6422::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
