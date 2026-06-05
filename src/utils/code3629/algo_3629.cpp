/**
 * @file algo_3629.cpp
 */
#include "code3629/algo_3629.h"
QVector<double> algo_3629::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
