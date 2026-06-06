/**
 * @file algo_7369.cpp
 */
#include "code7369/algo_7369.h"
QVector<double> algo_7369::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
