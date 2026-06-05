/**
 * @file algo_6549.cpp
 */
#include "code6549/algo_6549.h"
QVector<double> algo_6549::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
