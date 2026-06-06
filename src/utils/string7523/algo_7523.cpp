/**
 * @file algo_7523.cpp
 */
#include "string7523/algo_7523.h"
QVector<double> algo_7523::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
