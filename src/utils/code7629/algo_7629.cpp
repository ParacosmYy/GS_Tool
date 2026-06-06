/**
 * @file algo_7629.cpp
 */
#include "code7629/algo_7629.h"
QVector<double> algo_7629::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
