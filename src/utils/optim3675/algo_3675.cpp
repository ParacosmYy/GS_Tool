/**
 * @file algo_3675.cpp
 */
#include "optim3675/algo_3675.h"
QVector<double> algo_3675::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
