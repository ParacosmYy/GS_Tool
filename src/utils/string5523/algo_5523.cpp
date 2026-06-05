/**
 * @file algo_5523.cpp
 */
#include "string5523/algo_5523.h"
QVector<double> algo_5523::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
