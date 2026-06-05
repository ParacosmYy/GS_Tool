/**
 * @file algo_3296.cpp
 */
#include "geometry3296/algo_3296.h"
QVector<double> algo_3296::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
