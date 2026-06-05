/**
 * @file algo_4369.cpp
 */
#include "code4369/algo_4369.h"
QVector<double> algo_4369::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
