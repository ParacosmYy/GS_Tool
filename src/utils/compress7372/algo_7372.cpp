/**
 * @file algo_7372.cpp
 */
#include "compress7372/algo_7372.h"
QVector<double> algo_7372::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
