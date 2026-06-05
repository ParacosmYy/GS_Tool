/**
 * @file algo_5975.cpp
 */
#include "optim5975/algo_5975.h"
QVector<double> algo_5975::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
