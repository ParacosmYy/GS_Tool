/**
 * @file algo_4580.cpp
 */
#include "sort4580/algo_4580.h"
QVector<double> algo_4580::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
