/**
 * @file algo_6283.cpp
 */
#include "string6283/algo_6283.h"
QVector<double> algo_6283::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
