/**
 * @file algo_2829.cpp
 */
#include "code2829/algo_2829.h"
QVector<double> algo_2829::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
