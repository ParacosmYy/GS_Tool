/**
 * @file algo_5682.cpp
 */
#include "poly5682/algo_5682.h"
QVector<double> algo_5682::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
