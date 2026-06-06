/**
 * @file algo_7215.cpp
 */
#include "optim7215/algo_7215.h"
QVector<double> algo_7215::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
