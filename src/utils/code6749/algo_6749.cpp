/**
 * @file algo_6749.cpp
 */
#include "code6749/algo_6749.h"
QVector<double> algo_6749::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
