/**
 * @file algo_7121.cpp
 */
#include "interp7121/algo_7121.h"
QVector<double> algo_7121::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
