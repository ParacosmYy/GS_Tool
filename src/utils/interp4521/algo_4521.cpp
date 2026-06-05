/**
 * @file algo_4521.cpp
 */
#include "interp4521/algo_4521.h"
QVector<double> algo_4521::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
