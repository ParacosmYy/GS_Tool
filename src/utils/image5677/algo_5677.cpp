/**
 * @file algo_5677.cpp
 */
#include "image5677/algo_5677.h"
QVector<double> algo_5677::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
