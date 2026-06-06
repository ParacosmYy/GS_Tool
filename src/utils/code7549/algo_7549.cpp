/**
 * @file algo_7549.cpp
 */
#include "code7549/algo_7549.h"
QVector<double> algo_7549::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
