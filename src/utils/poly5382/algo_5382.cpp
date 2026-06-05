/**
 * @file algo_5382.cpp
 */
#include "poly5382/algo_5382.h"
QVector<double> algo_5382::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
