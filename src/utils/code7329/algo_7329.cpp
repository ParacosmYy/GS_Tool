/**
 * @file algo_7329.cpp
 */
#include "code7329/algo_7329.h"
QVector<double> algo_7329::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
