/**
 * @file algo_6736.cpp
 */
#include "geometry6736/algo_6736.h"
QVector<double> algo_6736::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
