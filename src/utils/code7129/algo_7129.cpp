/**
 * @file algo_7129.cpp
 */
#include "code7129/algo_7129.h"
QVector<double> algo_7129::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
