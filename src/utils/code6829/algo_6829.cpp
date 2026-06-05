/**
 * @file algo_6829.cpp
 */
#include "code6829/algo_6829.h"
QVector<double> algo_6829::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
