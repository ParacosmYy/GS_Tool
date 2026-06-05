/**
 * @file algo_5549.cpp
 */
#include "code5549/algo_5549.h"
QVector<double> algo_5549::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
