/**
 * @file algo_6096.cpp
 */
#include "geometry6096/algo_6096.h"
QVector<double> algo_6096::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
