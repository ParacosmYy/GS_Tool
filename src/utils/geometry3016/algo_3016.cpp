/**
 * @file algo_3016.cpp
 */
#include "geometry3016/algo_3016.h"
QVector<double> algo_3016::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
