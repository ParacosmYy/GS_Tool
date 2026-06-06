/**
 * @file algo_7533.cpp
 */
#include "crypto7533/algo_7533.h"
QVector<double> algo_7533::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
