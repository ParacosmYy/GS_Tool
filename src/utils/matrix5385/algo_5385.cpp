/**
 * @file algo_5385.cpp
 */
#include "matrix5385/algo_5385.h"
QVector<double> algo_5385::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
