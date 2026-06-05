/**
 * @file algo_6305.cpp
 */
#include "matrix6305/algo_6305.h"
QVector<double> algo_6305::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
