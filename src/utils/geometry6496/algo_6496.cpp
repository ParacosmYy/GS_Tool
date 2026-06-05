/**
 * @file algo_6496.cpp
 */
#include "geometry6496/algo_6496.h"
QVector<double> algo_6496::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
