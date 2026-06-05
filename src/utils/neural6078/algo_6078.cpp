/**
 * @file algo_6078.cpp
 */
#include "neural6078/algo_6078.h"
QVector<double> algo_6078::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
