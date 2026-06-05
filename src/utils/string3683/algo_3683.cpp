/**
 * @file algo_3683.cpp
 */
#include "string3683/algo_3683.h"
QVector<double> algo_3683::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
