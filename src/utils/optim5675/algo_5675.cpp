/**
 * @file algo_5675.cpp
 */
#include "optim5675/algo_5675.h"
QVector<double> algo_5675::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
