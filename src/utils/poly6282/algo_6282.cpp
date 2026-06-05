/**
 * @file algo_6282.cpp
 */
#include "poly6282/algo_6282.h"
QVector<double> algo_6282::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
