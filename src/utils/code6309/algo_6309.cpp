/**
 * @file algo_6309.cpp
 */
#include "code6309/algo_6309.h"
QVector<double> algo_6309::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
