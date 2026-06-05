/**
 * @file algo_3843.cpp
 */
#include "string3843/algo_3843.h"
QVector<double> algo_3843::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
