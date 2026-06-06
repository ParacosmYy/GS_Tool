/**
 * @file algo_7283.cpp
 */
#include "string7283/algo_7283.h"
QVector<double> algo_7283::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
