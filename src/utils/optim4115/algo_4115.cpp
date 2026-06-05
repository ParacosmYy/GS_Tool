/**
 * @file algo_4115.cpp
 */
#include "optim4115/algo_4115.h"
QVector<double> algo_4115::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
