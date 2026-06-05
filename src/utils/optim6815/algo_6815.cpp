/**
 * @file algo_6815.cpp
 */
#include "optim6815/algo_6815.h"
QVector<double> algo_6815::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
