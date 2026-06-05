/**
 * @file algo_3075.cpp
 */
#include "optim3075/algo_3075.h"
QVector<double> algo_3075::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
