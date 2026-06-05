/**
 * @file algo_6129.cpp
 */
#include "code6129/algo_6129.h"
QVector<double> algo_6129::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
