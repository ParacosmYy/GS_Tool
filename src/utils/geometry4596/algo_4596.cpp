/**
 * @file algo_4596.cpp
 */
#include "geometry4596/algo_4596.h"
QVector<double> algo_4596::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
