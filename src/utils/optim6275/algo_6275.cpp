/**
 * @file algo_6275.cpp
 */
#include "optim6275/algo_6275.h"
QVector<double> algo_6275::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
