/**
 * @file algo_5189.cpp
 */
#include "code5189/algo_5189.h"
QVector<double> algo_5189::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
